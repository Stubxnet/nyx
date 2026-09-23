#pragma once

#include <cstdint>
#include <memory>
#include <unordered_map>
#include <random>
#include <mutex>

#include "Entity.hpp"

class Entities {
public:
    using Uuid = std::int64_t;

    std::shared_ptr<Entity> GetEntity(Uuid uuid) const noexcept {
        auto it = entities.find(uuid);
        return it == entities.end() ? nullptr : it->second;
    }

    void DepopEntity(Uuid uuid) noexcept {
        std::cout << "Entity depoped uuid " << uuid << std::endl;
        entities.erase(uuid);
    }

    void DespawnEntity(Uuid uuid) noexcept {
        std::cout << "Entity despawned uuid " << uuid << std::endl;
        DepopEntity(uuid);
    }

    Uuid SpawnEntity(int64_t eid) {
        Uuid uuid = GenerateUuid();
        auto ent = std::make_shared<Entity>();
        ent->setEid(static_cast<int>(eid));
        entities.emplace(uuid, std::move(ent));
        std::cout << "Spawned entity with eid " << eid << " and uuid " << uuid << std::endl;
        return uuid;
    }

    Uuid DuplicateEntity(Uuid uuid) {
        auto original = GetEntity(uuid);
        if (!original) return 0;

        Uuid newUuid = GenerateUuid();
        auto copy = std::make_shared<Entity>(*original);
        entities.emplace(newUuid, std::move(copy));
        std::cout << "Duplicated entity with uuid " << uuid << " to uuid " << newUuid << std::endl;
        return newUuid;
    }

private:
    Uuid GenerateUuid() {
        thread_local std::mt19937_64 rng{std::random_device{}()};
        return static_cast<Uuid>(rng());
    }

    mutable std::mutex mtx;
    std::unordered_map<Uuid, std::shared_ptr<Entity>> entities;
};
