// Copyright (c) 2013-2016 mogemimi. Distributed under the MIT license.

#include <Pomdog.Experimental/Gameplay/Entity.hpp>
#include <gtest/iutest_switch.hpp>
#include <cstdint>
#include <memory>

using Pomdog::Component;
using Pomdog::Entity;
using Pomdog::EntityID;
using Pomdog::EntityContext;

namespace {

struct TransformComponent: public Component<TransformComponent>
{
    TransformComponent() = default;
    TransformComponent(int xIn, int yIn, int zIn)
        : x(xIn), y(yIn), z(zIn)
    {}

    int x, y, z;
};

struct PhysicsComponent: public Component<PhysicsComponent>
{
    PhysicsComponent() = default;
    explicit PhysicsComponent(int vIn)
        : v(vIn)
    {}

    int v;
};

struct RendererComponent: public Component<RendererComponent>
{
    virtual ~RendererComponent() = default;

    virtual void SetZOrder(int z) = 0;
    virtual int GetZOrder() const = 0;
};

struct MeshRendererComponent final: public RendererComponent
{
    void SetZOrder(int zIn) override {
        this->z = zIn;
    }
    int GetZOrder() const override {
        return z;
    }

private:
    int z;
};

}// unnamed namespace

TEST(Entity, AddComponentWithoutArguments)
{
    auto objectContext = std::make_shared<EntityContext>();
    Entity entity{objectContext};
    entity.AddComponent<TransformComponent>();
    entity.AddComponent<PhysicsComponent>();

    EXPECT_TRUE(entity.HasComponent<TransformComponent>());
    EXPECT_TRUE(entity.HasComponent<PhysicsComponent>());

    EXPECT_NE(nullptr, entity.GetComponent<TransformComponent>());
    EXPECT_NE(nullptr, entity.GetComponent<PhysicsComponent>());
}

TEST(Entity, AddComponentWithArguments)
{
    auto objectContext = std::make_shared<EntityContext>();
    Entity entity{objectContext};
    entity.AddComponent<TransformComponent>(3, 4, 5);
    entity.AddComponent<PhysicsComponent>(42);

    EXPECT_TRUE(entity.HasComponent<TransformComponent>());
    EXPECT_TRUE(entity.HasComponent<PhysicsComponent>());

    ASSERT_NE(nullptr, entity.GetComponent<TransformComponent>());
    ASSERT_NE(nullptr, entity.GetComponent<PhysicsComponent>());

    EXPECT_EQ(3, entity.GetComponent<TransformComponent>()->x);
    EXPECT_EQ(4, entity.GetComponent<TransformComponent>()->y);
    EXPECT_EQ(5, entity.GetComponent<TransformComponent>()->z);
    EXPECT_EQ(42, entity.GetComponent<PhysicsComponent>()->v);
}

TEST(Entity, AddComponentWithInheritance)
{
    auto objectContext = std::make_shared<EntityContext>();
    Entity entity{objectContext};
    auto meshRenderer = entity.AddComponent<MeshRendererComponent>(std::make_shared<MeshRendererComponent>());
    meshRenderer->SetZOrder(42);

    //EXPECT_TRUE(entity.HasComponent<MeshRendererComponent>());
    ASSERT_NE(nullptr, entity.GetComponent<MeshRendererComponent>());
    EXPECT_EQ(42, entity.GetComponent<MeshRendererComponent>()->GetZOrder());

    EXPECT_TRUE(entity.HasComponent<RendererComponent>());
    ASSERT_NE(nullptr, entity.GetComponent<RendererComponent>());
    auto renderer = entity.GetComponent<RendererComponent>();
    renderer->SetZOrder(72);

    EXPECT_EQ(72, entity.GetComponent<RendererComponent>()->GetZOrder());
    EXPECT_EQ(72, entity.GetComponent<MeshRendererComponent>()->GetZOrder());
}

TEST(Entity, RemoveComponent)
{
    {
        auto objectContext = std::make_shared<EntityContext>();
        Entity entity{objectContext};
        entity.AddComponent<TransformComponent>();
        ASSERT_TRUE(entity.HasComponent<TransformComponent>());

        entity.RemoveComponent<TransformComponent>();
        EXPECT_FALSE(entity.HasComponent<TransformComponent>());
        EXPECT_EQ(nullptr, entity.GetComponent<TransformComponent>());
    }
    {
        auto objectContext = std::make_shared<EntityContext>();
        Entity entity{objectContext};
        entity.AddComponent<TransformComponent>(3, 4, 5);
        entity.AddComponent<PhysicsComponent>(42);

        EXPECT_TRUE(entity.HasComponent<TransformComponent>());
        EXPECT_TRUE(entity.HasComponent<PhysicsComponent>());

        ASSERT_NE(nullptr, entity.GetComponent<TransformComponent>());
        ASSERT_NE(nullptr, entity.GetComponent<PhysicsComponent>());

        entity.RemoveComponent<TransformComponent>();
        EXPECT_FALSE(entity.HasComponent<TransformComponent>());
        EXPECT_EQ(nullptr, entity.GetComponent<TransformComponent>());

        entity.RemoveComponent<PhysicsComponent>();
        EXPECT_FALSE(entity.HasComponent<PhysicsComponent>());
        EXPECT_EQ(nullptr, entity.GetComponent<PhysicsComponent>());
    }
}

TEST(Entity, RemoveComponentWithInheritance)
{
    auto objectContext = std::make_shared<EntityContext>();
    {
        Entity entity{objectContext};
        entity.AddComponent<MeshRendererComponent>(std::make_shared<MeshRendererComponent>());

        //EXPECT_TRUE(entity.HasComponent<MeshRendererComponent>());
        EXPECT_NE(nullptr, entity.GetComponent<MeshRendererComponent>());
        EXPECT_TRUE(entity.HasComponent<RendererComponent>());
        EXPECT_NE(nullptr, entity.GetComponent<RendererComponent>());

        entity.RemoveComponent<MeshRendererComponent>();
        //EXPECT_FALSE(entity.HasComponent<MeshRendererComponent>());
        EXPECT_EQ(nullptr, entity.GetComponent<MeshRendererComponent>());
        EXPECT_FALSE(entity.HasComponent<RendererComponent>());
        EXPECT_EQ(nullptr, entity.GetComponent<RendererComponent>());
    }
    {
        Entity entity{objectContext};
        entity.AddComponent<MeshRendererComponent>(std::make_shared<MeshRendererComponent>());

        //EXPECT_TRUE(entity.HasComponent<MeshRendererComponent>());
        EXPECT_NE(nullptr, entity.GetComponent<MeshRendererComponent>());
        EXPECT_TRUE(entity.HasComponent<RendererComponent>());
        EXPECT_NE(nullptr, entity.GetComponent<RendererComponent>());

        entity.RemoveComponent<RendererComponent>();
        //EXPECT_FALSE(entity.HasComponent<MeshRendererComponent>());
        EXPECT_EQ(nullptr, entity.GetComponent<MeshRendererComponent>());
        EXPECT_FALSE(entity.HasComponent<RendererComponent>());
        EXPECT_EQ(nullptr, entity.GetComponent<RendererComponent>());
    }
}

TEST(Entity, Component_Const)
{
    auto objectContext = std::make_shared<EntityContext>();
    auto entity = std::make_shared<Entity>(objectContext);

    EXPECT_FALSE(entity->HasComponent<PhysicsComponent>());
    EXPECT_EQ(nullptr, entity->GetComponent<PhysicsComponent>());

    entity->AddComponent<PhysicsComponent>(42);

    EXPECT_TRUE(entity->HasComponent<PhysicsComponent>());
    ASSERT_NE(nullptr, entity->GetComponent<PhysicsComponent>());
    EXPECT_EQ(42, entity->GetComponent<PhysicsComponent>()->v);

    {
        std::shared_ptr<Entity const> entityConstRef = entity;
        EXPECT_TRUE(entityConstRef->HasComponent<PhysicsComponent>());
        ASSERT_NE(nullptr, entityConstRef->GetComponent<PhysicsComponent>());
        EXPECT_EQ(42, entityConstRef->GetComponent<PhysicsComponent>()->v);
    }
}

TEST(Entity, DestroyImmediate)
{
    {
        auto objectContext = std::make_shared<EntityContext>();
        Entity entity {objectContext};

        EXPECT_TRUE(entity);
        EXPECT_EQ(1, objectContext->Count());
        entity.DestroyImmediate();
        EXPECT_FALSE(entity);
        EXPECT_EQ(0, objectContext->Count());
    }
    {
        auto objectContext = std::make_shared<EntityContext>();
        Entity entity {objectContext};

        EXPECT_TRUE(entity);
        EXPECT_EQ(1, objectContext->Count());
        objectContext->DestroyImmediate(entity.ID());
        EXPECT_FALSE(entity);
        EXPECT_EQ(0, objectContext->Count());
    }
    {
        auto objectContext = std::make_shared<EntityContext>();
        Entity entity1 {objectContext};
        Entity entity2 {objectContext};
        Entity entity3 {objectContext};

        EXPECT_TRUE(entity1);
        EXPECT_TRUE(entity2);
        EXPECT_TRUE(entity3);
        EXPECT_EQ(3, objectContext->Count());
        entity1.DestroyImmediate();
        EXPECT_FALSE(entity1);
        EXPECT_TRUE(entity2);
        EXPECT_TRUE(entity3);
        EXPECT_EQ(2, objectContext->Count());
        entity3.DestroyImmediate();
        EXPECT_FALSE(entity1);
        EXPECT_TRUE(entity2);
        EXPECT_FALSE(entity3);
        EXPECT_EQ(1, objectContext->Count());
        entity2.DestroyImmediate();
        EXPECT_FALSE(entity1);
        EXPECT_FALSE(entity2);
        EXPECT_FALSE(entity3);
        EXPECT_EQ(0, objectContext->Count());
    }

    struct Behavior: public Pomdog::Component<Behavior> {
        void Do(Pomdog::Entity & self)
        {
            EXPECT_TRUE(self);
            EXPECT_TRUE(self.HasComponent<Behavior>());
            self.DestroyImmediate();
            EXPECT_FALSE(self);
            //EXPECT_FALSE(self.HasComponent<Behavior>());
        }

        std::shared_ptr<int> ptr = std::make_shared<int>(42);
    };

    {
        auto objectContext = std::make_shared<EntityContext>();
        Entity entity {objectContext};
        auto behavior = entity.AddComponent<Behavior>();
        std::weak_ptr<int> weak = behavior->ptr;

        EXPECT_TRUE(entity);
        EXPECT_TRUE(entity.HasComponent<Behavior>());
        EXPECT_EQ(behavior, entity.GetComponent<Behavior>());
        EXPECT_FALSE(weak.expired());
        EXPECT_EQ(1, objectContext->Count());
        behavior->Do(entity);
        EXPECT_FALSE(entity);
        //EXPECT_FALSE(entity.HasComponent<Behavior>());
        //EXPECT_FALSE(entity.GetComponent<Behavior>());
        EXPECT_TRUE(behavior);
        EXPECT_FALSE(weak.expired());
        behavior.reset();
        EXPECT_TRUE(weak.expired());
        EXPECT_EQ(0, objectContext->Count());
    }
}

TEST(Entity, Destroy)
{
    {
        auto objectContext = std::make_shared<EntityContext>();
        Entity entity {objectContext};

        EXPECT_TRUE(entity);
        EXPECT_EQ(1, objectContext->Count());
        entity.Destroy();
        EXPECT_FALSE(entity);
        EXPECT_EQ(0, objectContext->Count());
        objectContext->Refresh();
        EXPECT_FALSE(entity);
        EXPECT_EQ(0, objectContext->Count());
    }
    {
        auto objectContext = std::make_shared<EntityContext>();
        Entity entity {objectContext};

        EXPECT_TRUE(entity);
        EXPECT_EQ(1, objectContext->Count());
        objectContext->Destroy(entity.ID());
        EXPECT_FALSE(entity);
        EXPECT_EQ(0, objectContext->Count());
        objectContext->Refresh();
        EXPECT_FALSE(entity);
        EXPECT_EQ(0, objectContext->Count());
    }
    {
        auto objectContext = std::make_shared<EntityContext>();
        Entity entity1 {objectContext};
        Entity entity2 {objectContext};
        Entity entity3 {objectContext};

        EXPECT_TRUE(entity1);
        EXPECT_TRUE(entity2);
        EXPECT_TRUE(entity3);
        EXPECT_EQ(3, objectContext->Count());
        entity1.Destroy();
        EXPECT_FALSE(entity1);
        EXPECT_TRUE(entity2);
        EXPECT_TRUE(entity3);
        EXPECT_EQ(2, objectContext->Count());
        entity3.Destroy();
        EXPECT_FALSE(entity1);
        EXPECT_TRUE(entity2);
        EXPECT_FALSE(entity3);
        EXPECT_EQ(1, objectContext->Count());
        entity2.Destroy();
        EXPECT_FALSE(entity1);
        EXPECT_FALSE(entity2);
        EXPECT_FALSE(entity3);
        EXPECT_EQ(0, objectContext->Count());
        objectContext->Refresh();
        EXPECT_FALSE(entity1);
        EXPECT_FALSE(entity2);
        EXPECT_FALSE(entity3);
        EXPECT_EQ(0, objectContext->Count());
    }

    struct Behavior: public Pomdog::Component<Behavior> {
        void Do(Pomdog::Entity & self)
        {
            EXPECT_TRUE(self);
            EXPECT_TRUE(self.HasComponent<Behavior>());
            self.Destroy();
            EXPECT_FALSE(self);
            //EXPECT_FALSE(self.HasComponent<Behavior>());
        }

        std::shared_ptr<int> ptr = std::make_shared<int>(42);
    };

    {
        auto objectContext = std::make_shared<EntityContext>();
        Entity entity {objectContext};
        auto behavior = entity.AddComponent<Behavior>();
        std::weak_ptr<int> weak = behavior->ptr;

        EXPECT_TRUE(entity);
        EXPECT_TRUE(entity.HasComponent<Behavior>());
        EXPECT_EQ(behavior, entity.GetComponent<Behavior>());
        EXPECT_FALSE(weak.expired());
        EXPECT_EQ(1, objectContext->Count());
        behavior->Do(entity);
        EXPECT_FALSE(entity);
        //EXPECT_FALSE(entity.HasComponent<Behavior>());
        //EXPECT_EQ(nullptr, entity.GetComponent<Behavior>());
        EXPECT_FALSE(weak.expired());
        EXPECT_EQ(0, objectContext->Count());
        objectContext->Refresh();
        EXPECT_FALSE(entity);
        //EXPECT_FALSE(entity.HasComponent<Behavior>());
        //EXPECT_FALSE(entity.GetComponent<Behavior>());
        EXPECT_FALSE(weak.expired());
        EXPECT_TRUE(behavior);
        behavior.reset();
        EXPECT_TRUE(weak.expired());
        EXPECT_EQ(0, objectContext->Count());
    }
}

TEST(Entity, Clear)
{
    {
        auto objectContext = std::make_shared<EntityContext>();
        Entity entity1 {objectContext};
        Entity entity2 {objectContext};
        Entity entity3 {objectContext};

        EXPECT_TRUE(entity1);
        EXPECT_TRUE(entity2);
        EXPECT_TRUE(entity3);
        EXPECT_EQ(3, objectContext->Count());
        objectContext->Clear();
        EXPECT_FALSE(entity1);
        EXPECT_FALSE(entity2);
        EXPECT_FALSE(entity3);
        EXPECT_EQ(0, objectContext->Count());
    }
    {
        auto objectContext = std::make_shared<EntityContext>();
        Entity entity1 {objectContext};
        Entity entity2 {objectContext};
        Entity entity3 {objectContext};

        entity1.AddComponent<TransformComponent>();
        entity1.AddComponent<PhysicsComponent>();
        entity2.AddComponent<TransformComponent>();
        entity3.AddComponent<PhysicsComponent>();

        EXPECT_TRUE(entity1);
        EXPECT_TRUE(entity2);
        EXPECT_TRUE(entity3);
        EXPECT_NE(entity1.ID(), entity2.ID());
        EXPECT_NE(entity2.ID(), entity3.ID());
        EXPECT_EQ(3, objectContext->Count());
        objectContext->Clear();
        EXPECT_FALSE(entity1);
        EXPECT_FALSE(entity2);
        EXPECT_FALSE(entity3);
        EXPECT_EQ(0, objectContext->Count());
    }
    {
        auto objectContext = std::make_shared<EntityContext>();
        Entity entity {objectContext};
        auto oldId = entity.ID();

        EXPECT_TRUE(entity);
        EXPECT_EQ(1, objectContext->Count());
        objectContext->Clear();
        EXPECT_FALSE(entity);
        EXPECT_EQ(0, objectContext->Count());

        entity = Entity{objectContext};
        auto newId = entity.ID();
        EXPECT_NE(oldId, newId);
        EXPECT_NE(oldId.SequenceNumber(), newId.SequenceNumber());
        EXPECT_EQ(oldId.Index(), newId.Index());
    }

    struct Behavior: public Pomdog::Component<Behavior> {
        std::shared_ptr<int> ptr = std::make_shared<int>(42);
    };

    {
        auto objectContext = std::make_shared<EntityContext>();
        Entity entity {objectContext};
        auto behavior = entity.AddComponent<Behavior>();
        std::weak_ptr<int> weak = behavior->ptr;

        EXPECT_TRUE(entity);
        EXPECT_TRUE(entity.HasComponent<Behavior>());
        EXPECT_EQ(behavior, entity.GetComponent<Behavior>());
        EXPECT_FALSE(weak.expired());
        EXPECT_EQ(1, objectContext->Count());
        objectContext->Clear();
        EXPECT_FALSE(entity);
        EXPECT_TRUE(behavior);
        EXPECT_FALSE(weak.expired());
        behavior.reset();
        EXPECT_TRUE(weak.expired());
        EXPECT_EQ(0, objectContext->Count());
    }
    {
        auto objectContext = std::make_shared<EntityContext>();
        Entity entity1 {objectContext};
        Entity entity2 {objectContext};
        auto behavior = entity1.AddComponent<Behavior>();
        std::weak_ptr<int> weak = behavior->ptr;

        auto oldId1 = entity1.ID();
        auto oldId2 = entity2.ID();

        EXPECT_TRUE(entity1);
        EXPECT_TRUE(entity2);
        EXPECT_EQ(2, objectContext->Count());
        objectContext->Clear();
        EXPECT_FALSE(entity1);
        EXPECT_FALSE(entity2);
        EXPECT_EQ(0, objectContext->Count());

        entity1 = Entity{objectContext};
        entity2 = Entity{objectContext};

        EXPECT_NE(oldId1, oldId2);
        EXPECT_NE(oldId1, entity1.ID());
        EXPECT_NE(oldId2, entity2.ID());
        EXPECT_NE(oldId1.SequenceNumber(), entity1.ID().SequenceNumber());
        EXPECT_NE(oldId2.SequenceNumber(), entity2.ID().SequenceNumber());
        EXPECT_EQ(oldId1.Index(), entity1.ID().Index());
        EXPECT_EQ(oldId2.Index(), entity2.ID().Index());
    }
}

TEST(Entity, EntityID)
{
    auto objectContext = std::make_shared<EntityContext>();
    Entity entity {objectContext};

    auto id = entity.ID();
    EXPECT_EQ(id, entity.ID());

    Entity entity2 {objectContext};

    EXPECT_NE(entity.ID(), entity2.ID());
    EXPECT_EQ(0, entity.ID().Index());
    EXPECT_EQ(1, entity2.ID().Index());
    EXPECT_EQ(1, entity.ID().SequenceNumber());
    EXPECT_EQ(1, entity2.ID().SequenceNumber());
}

TEST(Entity, EntityID_Sequence)
{
    auto objectContext = std::make_shared<EntityContext>();
    {
        Entity entity {objectContext};
        EXPECT_EQ(0U, entity.ID().Index());
        EXPECT_NE(0U, entity.ID().SequenceNumber());
        EXPECT_EQ(1U, entity.ID().SequenceNumber());
    }
    {
        Entity entity {objectContext};
        EXPECT_EQ(1U, entity.ID().Index());
        EXPECT_NE(0U, entity.ID().SequenceNumber());
        EXPECT_EQ(1U, entity.ID().SequenceNumber());
        entity.DestroyImmediate();
    }
    {
        Entity entity {objectContext};
        EXPECT_EQ(1U, entity.ID().Index());
        EXPECT_NE(0U, entity.ID().SequenceNumber());
        EXPECT_EQ(2U, entity.ID().SequenceNumber());
        entity.DestroyImmediate();
    }
    {
        Entity entity {objectContext};
        EXPECT_EQ(1U, entity.ID().Index());
        EXPECT_NE(0U, entity.ID().SequenceNumber());
        EXPECT_EQ(3U, entity.ID().SequenceNumber());
        entity.DestroyImmediate();
    }
    {
        Entity entity1 {objectContext};
        EXPECT_EQ(1U, entity1.ID().Index());
        EXPECT_NE(0U, entity1.ID().SequenceNumber());
        EXPECT_EQ(4U, entity1.ID().SequenceNumber());

        Entity entity2 {objectContext};
        EXPECT_EQ(2U, entity2.ID().Index());
        EXPECT_NE(0U, entity2.ID().SequenceNumber());
        EXPECT_EQ(1U, entity2.ID().SequenceNumber());

        Entity entity3 {objectContext};
        EXPECT_EQ(3U, entity3.ID().Index());
        EXPECT_NE(0U, entity3.ID().SequenceNumber());
        EXPECT_EQ(1U, entity3.ID().SequenceNumber());

        entity1.DestroyImmediate();
        entity2.DestroyImmediate();
        entity3.DestroyImmediate();
    }
}

TEST(Entity, Cast_Bool)
{
    auto objectContext = std::make_shared<EntityContext>();
    {
        Entity entity {objectContext};
        EXPECT_TRUE(entity);
        entity.DestroyImmediate();
        EXPECT_FALSE(entity);
    }
    {
        Entity entity1 {objectContext};
        Entity entity2 {objectContext};
        EXPECT_TRUE(entity1);
        EXPECT_TRUE(entity2);
        entity1.DestroyImmediate();
        EXPECT_FALSE(entity1);
        EXPECT_TRUE(entity2);
        entity2.DestroyImmediate();
        EXPECT_FALSE(entity1);
        EXPECT_FALSE(entity2);
    }
    {
        Entity entity {objectContext};
        auto copiedObject = entity;
        EXPECT_TRUE(entity);
        EXPECT_TRUE(copiedObject);
        entity.DestroyImmediate();
        EXPECT_FALSE(entity);
        EXPECT_FALSE(copiedObject);
    }
    {
        Entity entity {objectContext};
        auto copiedObject = entity;
        EXPECT_TRUE(entity);
        EXPECT_TRUE(copiedObject);
        copiedObject.DestroyImmediate();
        EXPECT_FALSE(entity);
        EXPECT_FALSE(copiedObject);
    }
}

TEST(Entity, EntityID_Unique)
{
    std::vector<Entity> objects;
    std::vector<std::uint64_t> uniqueIdents;

    auto objectContext = std::make_shared<EntityContext>();

    std::mt19937 random;

    std::uint32_t maxSequenceNumber = 0;
    std::uint32_t maxIndex = 0;

    for (std::size_t count = 0; count < 262144; ++count)
    {
        if (count % 3 != 0)
        {
            Entity entity{objectContext};
            objects.push_back(entity);
            uniqueIdents.push_back(entity.ID().Value());

            //printf("### Create Object: %llu(%u, %u) \n",
            //    entity->ID().Value(),
            //    entity->ID().SequenceNumber(),
            //    entity->ID().Index());

            maxSequenceNumber = std::max(maxSequenceNumber, entity.ID().SequenceNumber());
            maxIndex = std::max(maxIndex, entity.ID().Index());
        }
        if (count % 11 == 8)
        {
            if (!objects.empty())
            {
                ASSERT_TRUE(objects.front());
                objects.front().DestroyImmediate();
                objects.erase(objects.begin());
                //printf("### Remove Object \n");
            }
        }
        if (count % 14 == 9)
        {
            if (!objects.empty())
            {
                std::uniform_int_distribution<std::uint32_t> distribution(1, 13);
                auto const randomNumber = distribution(random);

                for (auto & object: objects)
                {
                    ASSERT_TRUE(object);
                    if (object.ID().Value() % randomNumber == 0) {
                        object.DestroyImmediate();
                    }
                }
                objects.erase(std::remove_if(std::begin(objects), std::end(objects),
                    [&](Entity const& object) { return !object; }), std::end(objects));

                //printf("### Remove Objects \n");
            }
        }
    }

    //printf("## maxSequenceNumber = %u\n## maxIndex = %u\n", maxSequenceNumber, maxIndex);

    ASSERT_FALSE(uniqueIdents.empty());

    std::sort(std::begin(uniqueIdents), std::end(uniqueIdents));
    auto iter = std::adjacent_find(std::begin(uniqueIdents), std::end(uniqueIdents));

    EXPECT_EQ(std::end(uniqueIdents), iter);
}