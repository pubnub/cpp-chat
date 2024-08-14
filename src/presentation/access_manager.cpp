#include "access_manager.hpp"
#include "application/access_manager_service.hpp"

Pubnub::AccessManager::AccessManager(std::shared_ptr<const AccessManagerService> access_manager_service) :
    access_manager_service(access_manager_service)
{}

bool Pubnub::AccessManager::can_i(Pubnub::AccessManager::Permission permission, Pubnub::AccessManager::ResourceType resource_type, const Pubnub::String& resource_name) const 
{
    return this->access_manager_service->can_i(permission, resource_type, resource_name);
}
