#include "restrictions_service.hpp"
#include "application/chat_service.hpp"
#include "infra/pubnub.hpp"
#include "infra/entity_repository.hpp"
#include "const_values.hpp"
#include "message.hpp"
#include "chat_helpers.hpp"
#include "domain/json.hpp"

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

	//Lift restrictions
	if(!restrictions.ban && !restrictions.mute)
	{
		String remove_member_string = String("[{\"uuid\": {\"id\": \"") + user_id + String("\"}}]");
        {
            auto pubnub_handle = this->pubnub->lock();
            pubnub_handle->remove_members(restrictions_channel, remove_member_string);
        }
		String event_payload_string = String("{\"channelId\": \"") + restrictions_channel + String("\", \"restriction\": \"lifted\", \"reason\": \"") + restrictions.reason + String("\"}");
        chat_service_shared->emit_chat_event(pubnub_chat_event_type::PCET_MODERATION, user_id, event_payload_string);
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
	String event_payload_string = String("{\"channelId\": \"") + restrictions_channel + String("\", \"restriction\": \"lifted") + restriction_text + String("\", \"reason\": \"") + restrictions.reason + String("\"}");
    chat_service_shared->emit_chat_event(pubnub_chat_event_type::PCET_MODERATION, user_id, event_payload_string);
}

Restriction RestrictionsService::get_user_restrictions(const String& user_id, const String& channel_id, int limit, const String& start, const String& end) const {
    auto get_restrictions_response = [this, user_id, limit, start, end] {
        auto pubnub_handle = this->pubnub->lock();
        return pubnub_handle->get_memberships(user_id, "totalCount,custom", limit, "", "", "", "");
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

Restriction RestrictionsService::get_channel_restrictions(const String& user_id, const String& channel_id, int limit, const String& start, const String& end) const {
    String full_channel_id = INTERNAL_MODERATION_PREFIX + channel_id;

    auto get_restrictions_response = [this, full_channel_id, limit, start, end] {
        auto pubnub_handle = this->pubnub->lock();
        return pubnub_handle->get_channel_members(full_channel_id, "totalCount,custom", limit, "", "", "", "");
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

void RestrictionsService::report_user(const String& user_id, const String& reason) const {
    String payload = String("{\"reason\": \"") + reason + String("\", \"reportedUserId\": \"") + user_id + String("\"}");
    
    auto chat_service_shared = chat_service.lock();
    chat_service_shared->emit_chat_event(pubnub_chat_event_type::PCET_REPORT, INTERNAL_ADMIN_CHANNEL, payload);
}

void RestrictionsService::report_message(const Message& message, const String& reason) const
{
    Json payload_json = json::object();
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
    chat_service_shared->emit_chat_event(pubnub_chat_event_type::PCET_REPORT, INTERNAL_ADMIN_CHANNEL, payload_json.dump());
}
