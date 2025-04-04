#include "restrictions_service.hpp"
#include "application/chat_service.hpp"
#include "channel.hpp"
#include "domain/channel_entity.hpp"
#include "infra/pubnub.hpp"
#include "infra/entity_repository.hpp"
#include "const_values.hpp"
#include "message.hpp"
#include "chat_helpers.hpp"
#include "domain/json.hpp"
#include "application/channel_service.hpp"
#include "application/user_service.hpp"
#include "domain/restrictions.hpp"

using namespace Pubnub;
using json = nlohmann::json;

RestrictionsService::RestrictionsService(ThreadSafePtr<PubNub> pubnub, std::weak_ptr<ChatService> chat_service):
    pubnub(pubnub),
    chat_service(chat_service)
{}

void RestrictionsService::set_restrictions(const String& user_id, const String& channel_id, const Restriction& restrictions) const {
    if(user_id.empty())
    {
        throw std::invalid_argument("Failed to set restrictions, user_id is empty");
    }
    if(channel_id.empty())
    {
        throw std::invalid_argument("Failed to set restrictions, channel_id is empty");
    }
    
    auto chat_service_shared = chat_service.lock();


	//Restrictions are held in new channel with ID: PUBNUB_INTERNAL_MODERATION_{ChannelName}
	String restrictions_channel = INTERNAL_MODERATION_PREFIX + channel_id;

    try {
        if (auto chat_service_shared = chat_service.lock())
        {
            chat_service_shared->channel_service->create_channel(restrictions_channel, ChannelEntity{});
        } else {
            throw std::runtime_error("Chat service is not available");
        }
    }
    catch (const std::invalid_argument& e)
    {
        // TODO: better error handling
        if (String(e.what()).find("already exists") == String::npos) {
            throw e;
        }
        // else ignore - moderation channel already exist
    }

    //Channel for moderation events is "INTERNAL_MODERATION_PREFIX.{user_id}" (INTERNAL_MODERATION_PREFIX without last _)
    String moderation_event_channel = INTERNAL_MODERATION_PREFIX.substring(0, INTERNAL_MODERATION_PREFIX.length() - 1) + Pubnub::String(".") + user_id;

	//Lift restrictions
	if(!restrictions.ban && !restrictions.mute)
	{
        {
            auto pubnub_handle = this->pubnub->lock();
            pubnub_handle->remove_members(restrictions_channel, Restrictions::remove_member_payload(user_id));
        }
		String event_payload_string = Restrictions::lift_restrictions_payload(restrictions_channel, restrictions.reason);
    
        chat_service_shared->emit_chat_event(pubnub_chat_event_type::PCET_MODERATION, moderation_event_channel, event_payload_string);
		return;
	}

	//Ban or mute the user
	String params_string = String("{\"ban\": ") + bool_to_string(restrictions.ban) + String(", \"mute\": ") + bool_to_string(restrictions.mute) + String(", \"reason\": \"") + restrictions.reason + String("\"}");
    {
        auto pubnub_handle = this->pubnub->lock();
        pubnub_handle->set_members(restrictions_channel, create_set_members_object(user_id, params_string));
    }
    String restriction_text;
    restrictions.ban ? restriction_text = "banned" : "muted";
	String event_payload_string = String("{\"channelId\": \"") + restrictions_channel + String("\", \"restriction\": \"") + restriction_text + String("\", \"reason\": \"") + restrictions.reason + String("\"}");
    chat_service_shared->emit_chat_event(pubnub_chat_event_type::PCET_MODERATION, moderation_event_channel, event_payload_string);
}

Restriction RestrictionsService::get_user_restrictions(const String& user_id, const String& channel_id, const Pubnub::String &sort, int limit, const Pubnub::Page &page) const {
    //TODO:: add filter here
    auto get_restrictions_response = [this, user_id, limit, sort, page] {
        auto pubnub_handle = this->pubnub->lock();
        return pubnub_handle->get_memberships(user_id, "totalCount,custom", limit, "", sort, page.next, page.prev);
    }();
    json response_json = json::parse(get_restrictions_response);

    if(response_json.is_null())
    {
        throw std::runtime_error("can't get user restrictions, response is incorrect");
    }

    json response_data_json = response_json["data"];
    String full_channel_id = INTERNAL_MODERATION_PREFIX + channel_id;
    Restriction FinalRestrictionsData;

   for (auto& element : response_data_json)
   {
        //Find restrictions data for requested channel
        if(String(element["channel"]["id"]) == full_channel_id)
        {
            if(element["custom"]["ban"] == true)
            {
                FinalRestrictionsData.ban = true;
            }
            if(element["custom"]["mute"] == true)
            {
                FinalRestrictionsData.mute = true;
            }
            FinalRestrictionsData.reason = String(element["custom"]["reason"]);
            break;
        }
   }

   return FinalRestrictionsData;
}

std::tuple<std::vector<Pubnub::UserRestriction>, Pubnub::Page, int, Pubnub::String> RestrictionsService::get_users_restrictions(const Pubnub::String &channel_id, const Pubnub::String &sort, int limit, const Pubnub::Page &page) const
{
    if(limit < 0 || limit > PN_MAX_LIMIT)
    {
        throw std::invalid_argument("can't get users restrictions, limit has to be within 0 - " + std::to_string(PN_MAX_LIMIT) + " range");
    }

    String full_channel_id = INTERNAL_MODERATION_PREFIX + channel_id;

    auto get_restrictions_response = [this, limit, sort, page, full_channel_id] {
        auto pubnub_handle = this->pubnub->lock();
        return pubnub_handle->get_channel_members(full_channel_id, "totalCount,custom", limit, "", sort, page.next, page.prev);
    }();

    Json response_json = Json::parse(get_restrictions_response);

    if(response_json.is_null())
    {
        throw std::runtime_error("can't get users restrictions, response is incorrect");
    }

    Json response_data_json = response_json["data"];
    std::vector<UserRestriction> final_restrictions;

    for (Json::Iterator single_data_json = response_data_json.begin(); single_data_json != response_data_json.end(); ++single_data_json)
    {

        if(!single_data_json.value().contains("custom") || single_data_json.value()["custom"].is_null())
        {
            continue;
        }

        Json custom_json = single_data_json.value()["custom"];

        if (custom_json.contains("ban") || custom_json.contains("mute"))
        {
            UserRestriction restriction;
            restriction.ban = custom_json.get_bool("ban").value_or(false);
            restriction.ban = custom_json.get_bool("mute").value_or(false);
            restriction.reason = custom_json.get_string("reason").value_or(String(""));
            restriction.user_id = single_data_json.value()["uuid"]["id"];

            final_restrictions.push_back(restriction);
        }
    }

    int total_count = response_json.get_int("totalCount").value_or(0);
    Page page_response({response_json.get_string("next").value_or(String("")), response_json.get_string("prev").value_or(String(""))});
    String status = response_json.get_string("status").value_or(String(""));
    std::tuple<std::vector<Pubnub::UserRestriction>, Pubnub::Page, int, Pubnub::String> return_tuple = std::make_tuple(final_restrictions, page_response, total_count, status);
    return return_tuple;
}

Restriction RestrictionsService::get_channel_restrictions(const String& user_id, const String& channel_id, const Pubnub::String &sort, int limit, const Pubnub::Page &page) const {
    if(limit < 0 || limit > PN_MAX_LIMIT)
    {
        throw std::invalid_argument("can't get channels restrictions, limit has to be within 0 - " + std::to_string(PN_MAX_LIMIT) + " range");
    }
    
    String full_channel_id = INTERNAL_MODERATION_PREFIX + channel_id;

    auto get_restrictions_response = [this, full_channel_id, limit, sort, page] {
        auto pubnub_handle = this->pubnub->lock();
        return pubnub_handle->get_channel_members(full_channel_id, "totalCount,custom", limit, "", sort, page.next, page.prev);
    }();

    json response_json = json::parse(get_restrictions_response);

    if(response_json.is_null())
    {
        throw std::runtime_error("can't get channel restrictions, response is incorrect");
    }

    json response_data_json = response_json["data"];
    Restriction FinalRestrictionsData;

   for (auto& element : response_data_json)
   {
        //Find restrictions data for requested channel
        if(String(element["uuid"]["id"]) == user_id)
        {
            if(element["custom"]["ban"] == true)
            {
                FinalRestrictionsData.ban = true;
            }
            if(element["custom"]["mute"] == true)
            {
                FinalRestrictionsData.mute = true;
            }
            FinalRestrictionsData.reason = String(element["custom"]["reason"]);
            break;
        }
   }

   return FinalRestrictionsData;
}

std::tuple<std::vector<Pubnub::ChannelRestriction>, Pubnub::Page, int, Pubnub::String> RestrictionsService::get_channels_restrictions(const Pubnub::String &user_id, const Pubnub::String &sort, int limit, const Pubnub::Page &page) const
{

    auto get_restrictions_response = [this, user_id, limit, sort, page] {
        auto pubnub_handle = this->pubnub->lock();
        String filter = "channel.id LIKE \"" + INTERNAL_MODERATION_PREFIX + "*\"";
        return pubnub_handle->get_memberships(user_id, "totalCount,custom", limit, "", sort, page.next, page.prev);
    }();

    Json response_json = Json::parse(get_restrictions_response);

    if(response_json.is_null())
    {
        throw std::runtime_error("can't get channel restrictions, response is incorrect");
    }

    Json response_data_json = response_json["data"];
    std::vector<ChannelRestriction> final_restrictions;

    for (Json::Iterator single_data_json = response_data_json.begin(); single_data_json != response_data_json.end(); ++single_data_json)
    {

        if(!single_data_json.value().contains("custom") || single_data_json.value()["custom"].is_null())
        {
            continue;
        }

        Json custom_json = single_data_json.value()["custom"];

        if (custom_json.contains("ban") || custom_json.contains("mute"))
        {
            ChannelRestriction restriction;
            restriction.ban = custom_json.get_bool("ban").value_or(false);
            restriction.ban = custom_json.get_bool("mute").value_or(false);
            restriction.reason = custom_json.get_string("reason").value_or(String(""));
            String final_channel_id = single_data_json.value()["channel"]["id"];
            final_channel_id.erase(0, INTERNAL_MODERATION_PREFIX.length());
            restriction.channel_id = final_channel_id;
            final_restrictions.push_back(restriction);
        }
    }

    int total_count = response_json.get_int("totalCount").value_or(0);
    Page page_response({response_json.get_string("next").value_or(String("")), response_json.get_string("prev").value_or(String(""))});
    String status = response_json.get_string("status").value_or(String(""));
    std::tuple<std::vector<Pubnub::ChannelRestriction>, Pubnub::Page, int, Pubnub::String> return_tuple = std::make_tuple(final_restrictions, page_response, total_count, status);
    return return_tuple;
}

void RestrictionsService::report_user(const String& user_id, const String& reason) const {
    String payload = String("{\"reason\": \"") + reason + String("\", \"reportedUserId\": \"") + user_id + String("\"}");
    
    auto chat_service_shared = chat_service.lock();
    chat_service_shared->emit_chat_event(pubnub_chat_event_type::PCET_REPORT, INTERNAL_ADMIN_CHANNEL, payload);
}

void RestrictionsService::report_message(const Message& message, const String& reason) const
{
    Json payload_json = json::object();
    String channel = INTERNAL_MODERATION_PREFIX + message.message_data().channel_id;
    payload_json.insert_or_update("text", message.text().c_str());
    payload_json.insert_or_update("reason", reason.c_str());
    payload_json.insert_or_update("reportedMessageChannelId", message.message_data().channel_id.c_str());
    payload_json.insert_or_update("reportedMessageTimetoken", message.timetoken().c_str());
    //TODO:: remove this if statement when get_history is fixed and returns message user's id correctly;
    if (!message.message_data().user_id.empty())
    {
        payload_json.insert_or_update("reportedUserId", message.message_data().user_id.c_str());
    }
    
    
    auto chat_service_shared = chat_service.lock();
    chat_service_shared->emit_chat_event(pubnub_chat_event_type::PCET_REPORT, channel, payload_json.dump());
}
