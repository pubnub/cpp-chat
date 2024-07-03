#include "message_service.hpp"
#include "infra/pubnub.hpp"
#include "infra/entity_repository.hpp"

MessageService::MessageService(ThreadSafePtr<PubNub> pubnub, std::shared_ptr<EntityRepository> entity_repository):
    pubnub(pubnub),
    entity_repository(entity_repository)
{}

