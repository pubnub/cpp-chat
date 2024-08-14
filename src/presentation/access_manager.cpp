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
