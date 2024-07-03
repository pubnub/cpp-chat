#include "user_service.hpp"
#include "infra/pubnub.hpp"
#include "infra/entity_repository.hpp"

UserService::UserService(ThreadSafePtr<PubNub> pubnub, std::shared_ptr<EntityRepository> entity_repository):
    pubnub(pubnub),
    entity_repository(entity_repository)
{}


UserEntity UserService::create_domain_from_presentation_data(Pubnub::String user_id, Pubnub::ChatUserData &presentation_data)
{
    UserEntity new_user_entity;
    new_user_entity.user_id = user_id;
    new_user_entity.user_name = presentation_data.user_name;
    new_user_entity.external_id = presentation_data.external_id;
    new_user_entity.profile_url = presentation_data.profile_url;
    new_user_entity.email = presentation_data.email;
    new_user_entity.custom_data_json = presentation_data.custom_data_json;
    new_user_entity.status = presentation_data.status;
    new_user_entity.type = presentation_data.type;

    return new_user_entity;
}
