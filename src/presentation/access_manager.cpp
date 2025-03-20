#include "access_manager.hpp"
#include "application/access_manager_service.hpp"

Pubnub::AccessManager::AccessManager(std::shared_ptr<const AccessManagerService> access_manager_service) :
    access_manager_service(access_manager_service)
{}

Pubnub::AccessManager::AccessManager(const Pubnub::AccessManager& other) :
access_manager_service(other.access_manager_service)
{}



Pubnub::AccessManager& Pubnub::AccessManager::operator =(const Pubnub::AccessManager& other) {
    if(this == &other)
    {
        return *this;
    }

    this->access_manager_service = other.access_manager_service;
    return *this;
}

bool Pubnub::AccessManager::can_i(Pubnub::AccessManager::Permission permission, Pubnub::AccessManager::ResourceType resource_type, const Pubnub::String& resource_name) const 
{
    return this->access_manager_service->can_i(permission, resource_type, resource_name);
}

void Pubnub::AccessManager::set_secret_key(const Pubnub::String key) const
{
	return this->access_manager_service->set_secret_key(key);
}

Pubnub::String Pubnub::AccessManager::parse_token(const Pubnub::String token) const
{
    return this->access_manager_service->parse_token(token);
}

void Pubnub::AccessManager::set_auth_token(const Pubnub::String token) const
{
    this->access_manager_service->set_auth_token(token);
}

int Pubnub::AccessManager::set_pubnub_origin(const Pubnub::String origin) const
{
    return this->access_manager_service->set_pubnub_origin(origin);
}

Pubnub::String Pubnub::AccessManager::grant_token(const Pubnub::String permission_object) const
{
    return this->access_manager_service->grant_token(permission_object);
}

Pubnub::String Pubnub::AccessManager::grant_token(Pubnub::GrantTokenPermissionObject permission_object)
{
    return this->access_manager_service->grant_token(this->access_manager_service->grant_token_permission_struct_to_string(permission_object));
}