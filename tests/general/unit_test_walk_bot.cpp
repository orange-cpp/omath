// Unit tests for omath::pathfinding::WalkBot
// Covers all status transitions, callback behaviour, and a full walk simulation.
#include <gtest/gtest.h>
#include "omath/pathfinding/walk_bot.hpp"
#include "omath/pathfinding/navigation_mesh.hpp"

using namespace omath;
using namespace omath::pathfinding;

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

// Build a simple bidirectional linear chain:
//   (0,0,0) <-> (1,0,0) <-> (2,0,0) <-> ... <-> (n-1,0,0)
static std::shared_ptr<NavigationMesh> make_linear_mesh(int n)
{
    auto mesh = std::make_shared<NavigationMesh>();
    for (int i = 0; i < n; ++i)
    {
        const Vector3<float> v{static_cast<float>(i), 0.f, 0.f};
        std::vector<Vector3<float>> neighbors;
        if (i > 0)
            neighbors.push_back(Vector3<float>{static_cast<float>(i - 1), 0.f, 0.f});
        if (i + 1 < n)
            neighbors.push_back(Vector3<float>{static_cast<float>(i + 1), 0.f, 0.f});
        mesh->m_vertex_map[v] = neighbors;
    }
    return mesh;
}

// Collect every status update fired during one update() call.
static auto make_status_recorder(std::vector<WalkBotStatus>& out)
{
    return [&out](WalkBotStatus s) { out.push_back(s); };
}

// Collect every "next node" hint fired during one update() call.
static auto make_node_recorder(std::vector<Vector3<float>>& out)
{
    return [&out](const Vector3<float>& v) { out.push_back(v); };
}

// ---------------------------------------------------------------------------
// Construction
// ---------------------------------------------------------------------------

TEST(WalkBotTests, DefaultConstructedBotIsInert)
{
    // A default-constructed bot with no mesh, no target, and no callbacks must
    // not crash.
    WalkBot bot;
    EXPECT_NO_THROW(bot.update({0.f, 0.f, 0.f}));
}

TEST(WalkBotTests, ConstructWithMeshAndDistance)
{
    auto mesh = make_linear_mesh(3);
    EXPECT_NO_THROW(WalkBot bot(mesh, 0.5f));
}

// ---------------------------------------------------------------------------
// Status: FINISHED
// ---------------------------------------------------------------------------

TEST(WalkBotTests, FiresFinishedWhenBotIsAtTarget)
{
    auto mesh = make_linear_mesh(3);
    WalkBot bot(mesh, 0.5f);
    bot.set_target({2.f, 0.f, 0.f});

    std::vector<WalkBotStatus> statuses;
    bot.on_status(make_status_recorder(statuses));

    // bot_position == target_position -> distance == 0, well within threshold
    bot.update({2.f, 0.f, 0.f});

    ASSERT_FALSE(statuses.empty());
    EXPECT_EQ(statuses.front(), WalkBotStatus::FINISHED);
}

TEST(WalkBotTests, FiresFinishedWhenBotIsWithinMinDistance)
{
    auto mesh = make_linear_mesh(3);
    WalkBot bot(mesh, 0.5f);
    bot.set_target({0.4f, 0.f, 0.f});

    std::vector<WalkBotStatus> statuses;
    bot.on_status(make_status_recorder(statuses));

    // distance between (0,0,0) and (0.4,0,0) is 0.4 < 0.5 threshold
    bot.update({0.f, 0.f, 0.f});

    ASSERT_FALSE(statuses.empty());
    EXPECT_EQ(statuses.front(), WalkBotStatus::FINISHED);
}

TEST(WalkBotTests, NoFinishedWhenOutsideMinDistance)
{
    auto mesh = make_linear_mesh(5);
    WalkBot bot(mesh, 0.5f);
    bot.set_target({4.f, 0.f, 0.f});

    std::vector<WalkBotStatus> statuses;
    bot.on_status(make_status_recorder(statuses));

    // Attach path callback so we get further status updates
    std::vector<Vector3<float>> nodes;
    bot.on_path(make_node_recorder(nodes));

    bot.update({0.f, 0.f, 0.f});

    // FINISHED must NOT appear in the status list
    for (auto s : statuses)
        EXPECT_NE(s, WalkBotStatus::FINISHED);
}

TEST(WalkBotTests, FinishedFiredEvenWithoutPathCallback)
{
    // FINISHED is emitted before the on_path guard, so it fires regardless.
    auto mesh = make_linear_mesh(2);
    WalkBot bot(mesh, 0.5f);
    bot.set_target({1.f, 0.f, 0.f});

    std::vector<WalkBotStatus> statuses;
    bot.on_status(make_status_recorder(statuses));
    // Intentionally do NOT register on_path callback.

    bot.update({1.f, 0.f, 0.f});

    ASSERT_EQ(statuses.size(), 1u);
    EXPECT_EQ(statuses[0], WalkBotStatus::FINISHED);
}

TEST(WalkBotTests, FinishedDoesNotFallThroughToPathing)
{
    // update() must return after FINISHED — PATHING must not fire on the same tick.
    auto mesh = make_linear_mesh(3);
    WalkBot bot(mesh, 0.5f);
    bot.set_target({1.f, 0.f, 0.f});

    std::vector<WalkBotStatus> statuses;
    bot.on_status(make_status_recorder(statuses));
    std::vector<Vector3<float>> nodes;
    bot.on_path(make_node_recorder(nodes));

    bot.update({1.f, 0.f, 0.f}); // bot is at target

    ASSERT_EQ(statuses.size(), 1u);
    EXPECT_EQ(statuses[0], WalkBotStatus::FINISHED);
    EXPECT_TRUE(nodes.empty());
}

// ---------------------------------------------------------------------------
// No target set
// ---------------------------------------------------------------------------

TEST(WalkBotTests, NoUpdateWithoutTarget)
{
    auto mesh = make_linear_mesh(3);
    WalkBot bot(mesh, 0.5f);
    // Intentionally do NOT call set_target()

    std::vector<WalkBotStatus> statuses;
    bot.on_status(make_status_recorder(statuses));
    std::vector<Vector3<float>> nodes;
    bot.on_path(make_node_recorder(nodes));

    bot.update({0.f, 0.f, 0.f});

    EXPECT_TRUE(statuses.empty());
    EXPECT_TRUE(nodes.empty());
}

// ---------------------------------------------------------------------------
// Status: IDLE — no path callback registered
// ---------------------------------------------------------------------------

TEST(WalkBotTests, NoPathCallbackMeansNoPathingStatus)
{
    auto mesh = make_linear_mesh(4);
    WalkBot bot(mesh, 0.5f);
    bot.set_target({3.f, 0.f, 0.f});

    std::vector<WalkBotStatus> statuses;
    bot.on_status(make_status_recorder(statuses));
    // No on_path registered -> update returns early after FINISHED check

    bot.update({0.f, 0.f, 0.f});

    EXPECT_TRUE(statuses.empty());
}

// ---------------------------------------------------------------------------
// Status: IDLE — null/expired navigation mesh
// ---------------------------------------------------------------------------

TEST(WalkBotTests, FiresIdleWhenNavMeshIsNull)
{
    WalkBot bot; // no mesh assigned
    bot.set_target({5.f, 0.f, 0.f});

    std::vector<WalkBotStatus> statuses;
    bot.on_status(make_status_recorder(statuses));

    std::vector<Vector3<float>> nodes;
    bot.on_path(make_node_recorder(nodes));

    bot.update({0.f, 0.f, 0.f});

    ASSERT_FALSE(statuses.empty());
    EXPECT_EQ(statuses.back(), WalkBotStatus::IDLE);
    EXPECT_TRUE(nodes.empty());
}

TEST(WalkBotTests, FiresIdleWhenNavMeshExpires)
{
    auto mesh = make_linear_mesh(4);
    WalkBot bot(mesh, 0.5f);
    bot.set_target({3.f, 0.f, 0.f});

    std::vector<WalkBotStatus> statuses;
    bot.on_status(make_status_recorder(statuses));

    std::vector<Vector3<float>> nodes;
    bot.on_path(make_node_recorder(nodes));

    // Let the shared_ptr expire — WalkBot holds only a weak_ptr.
    mesh.reset();

    bot.update({0.f, 0.f, 0.f});

    ASSERT_FALSE(statuses.empty());
    EXPECT_EQ(statuses.back(), WalkBotStatus::IDLE);
    EXPECT_TRUE(nodes.empty());
}

TEST(WalkBotTests, SetNavMeshRestoresPathing)
{
    WalkBot bot; // starts with no mesh
    bot.set_target({3.f, 0.f, 0.f});

    std::vector<WalkBotStatus> statuses;
    bot.on_status(make_status_recorder(statuses));
    std::vector<Vector3<float>> nodes;
    bot.on_path(make_node_recorder(nodes));

    // First call — no mesh -> IDLE
    bot.update({0.f, 0.f, 0.f});
    ASSERT_EQ(statuses.back(), WalkBotStatus::IDLE);

    // Assign a mesh and call again. Keep the shared_ptr alive so the
    // weak_ptr inside WalkBot does not expire before update() is called.
    statuses.clear();
    nodes.clear();
    auto new_mesh = make_linear_mesh(4);
    bot.set_nav_mesh(new_mesh);
    bot.set_min_node_distance(0.5f);
    bot.update({0.f, 0.f, 0.f});

    ASSERT_FALSE(statuses.empty());
    EXPECT_EQ(statuses.back(), WalkBotStatus::PATHING);
    EXPECT_FALSE(nodes.empty());
}

// ---------------------------------------------------------------------------
// Status: IDLE — A* finds no path
// ---------------------------------------------------------------------------

TEST(WalkBotTests, FiresIdleWhenNoPathExists)
{
    // Disconnected graph: two isolated vertices
    auto mesh = std::make_shared<NavigationMesh>();
    mesh->m_vertex_map[{0.f, 0.f, 0.f}] = {};
    mesh->m_vertex_map[{10.f, 0.f, 0.f}] = {};

    WalkBot bot(mesh, 0.5f);
    bot.set_target({10.f, 0.f, 0.f});

    std::vector<WalkBotStatus> statuses;
    bot.on_status(make_status_recorder(statuses));
    std::vector<Vector3<float>> nodes;
    bot.on_path(make_node_recorder(nodes));

    bot.update({0.f, 0.f, 0.f});

    ASSERT_FALSE(statuses.empty());
    EXPECT_EQ(statuses.back(), WalkBotStatus::IDLE);
    EXPECT_TRUE(nodes.empty());
}

// ---------------------------------------------------------------------------
// Status: PATHING — normal routing
// ---------------------------------------------------------------------------

TEST(WalkBotTests, FiresPathingAndProvidesNextNode)
{
    auto mesh = make_linear_mesh(4);
    WalkBot bot(mesh, 0.5f);
    bot.set_target({3.f, 0.f, 0.f});

    std::vector<WalkBotStatus> statuses;
    bot.on_status(make_status_recorder(statuses));
    std::vector<Vector3<float>> nodes;
    bot.on_path(make_node_recorder(nodes));

    bot.update({0.f, 0.f, 0.f});

    ASSERT_FALSE(statuses.empty());
    EXPECT_EQ(statuses.back(), WalkBotStatus::PATHING);
    ASSERT_FALSE(nodes.empty());
}

TEST(WalkBotTests, NextNodeIsOnThePath)
{
    auto mesh = make_linear_mesh(5);
    WalkBot bot(mesh, 0.5f);
    bot.set_target({4.f, 0.f, 0.f});

    std::vector<Vector3<float>> nodes;
    bot.on_path(make_node_recorder(nodes));

    bot.update({0.f, 0.f, 0.f});

    // The suggested node must be a mesh vertex (x in [0..4], y=0, z=0)
    ASSERT_FALSE(nodes.empty());
    const auto& hint = nodes.front();
    EXPECT_GE(hint.x, 0.f);
    EXPECT_LE(hint.x, 4.f);
    EXPECT_FLOAT_EQ(hint.y, 0.f);
    EXPECT_FLOAT_EQ(hint.z, 0.f);
}

// ---------------------------------------------------------------------------
// set_min_node_distance
// ---------------------------------------------------------------------------

TEST(WalkBotTests, MinNodeDistanceAffectsFinishedThreshold)
{
    auto mesh = make_linear_mesh(3);
    WalkBot bot(mesh, 0.1f); // very tight threshold
    bot.set_target({1.f, 0.f, 0.f});

    std::vector<WalkBotStatus> statuses;
    bot.on_status(make_status_recorder(statuses));

    // distance 0.4 — outside 0.1 threshold
    bot.update({0.6f, 0.f, 0.f});
    EXPECT_TRUE(statuses.empty() ||
                std::find(statuses.begin(), statuses.end(), WalkBotStatus::FINISHED) == statuses.end());

    statuses.clear();
    bot.set_min_node_distance(0.5f); // widen threshold
    bot.update({0.6f, 0.f, 0.f});   // now 0.4 < 0.5 -> FINISHED

    ASSERT_FALSE(statuses.empty());
    EXPECT_EQ(statuses.front(), WalkBotStatus::FINISHED);
}

// ---------------------------------------------------------------------------
// reset()
// ---------------------------------------------------------------------------

TEST(WalkBotTests, ResetClearsLastVisited)
{
    auto mesh = make_linear_mesh(3);
    WalkBot bot(mesh, 0.5f);
    bot.set_target({2.f, 0.f, 0.f});

    std::vector<Vector3<float>> nodes;
    bot.on_path(make_node_recorder(nodes));

    // Tick 1: mark node 0 visited -> hint is node 1
    bot.update({0.05f, 0.f, 0.f});
    ASSERT_FALSE(nodes.empty());
    EXPECT_FLOAT_EQ(nodes.back().x, 1.f);

    // Without reset, a second tick from the same position also gives node 1.
    nodes.clear();
    bot.reset();

    // After reset, m_last_visited is cleared. The nearest node is 0 again,
    // it is within threshold so it gets marked visited and we advance to 1.
    // The hint should still be node 1 (same outcome, but state was cleanly reset).
    bot.update({0.05f, 0.f, 0.f});
    ASSERT_FALSE(nodes.empty());
    // Confirm the bot still routes correctly after reset.
    EXPECT_GE(nodes.back().x, 0.f);
}

// ---------------------------------------------------------------------------
// Node advancement — visited node causes skip to next
// ---------------------------------------------------------------------------

TEST(WalkBotTests, AdvancesWhenNearestNodeAlreadyVisited)
{
    // Chain: (0,0,0) -> (1,0,0) -> (2,0,0)
    auto mesh = make_linear_mesh(3);
    WalkBot bot(mesh, 0.5f);
    bot.set_target({2.f, 0.f, 0.f});

    std::vector<Vector3<float>> nodes;
    bot.on_path(make_node_recorder(nodes));

    // Tick 1: bot is very close to node 0 -> node 0 marked visited -> hint is node 1.
    bot.update({0.05f, 0.f, 0.f});
    ASSERT_FALSE(nodes.empty());
    EXPECT_FLOAT_EQ(nodes.back().x, 1.f);

    nodes.clear();

    // Tick 2: bot has moved to near node 1 -> node 1 marked visited -> hint advances to node 2.
    bot.update({1.05f, 0.f, 0.f});
    ASSERT_FALSE(nodes.empty());
    EXPECT_GT(nodes.back().x, 1.f);
}

// ---------------------------------------------------------------------------
// Displacement recovery — bot knocked back to unvisited node
// ---------------------------------------------------------------------------

TEST(WalkBotTests, RecoverAfterDisplacementToUnvisitedNode)
{
    // Chain: 0 -> 1 -> 2 -> 3 -> 4
    auto mesh = make_linear_mesh(5);
    WalkBot bot(mesh, 0.5f);
    bot.set_target({4.f, 0.f, 0.f});

    std::vector<Vector3<float>> nodes;
    bot.on_path(make_node_recorder(nodes));

    // Walk forward through nodes 0-3 to build visited state.
    for (int i = 0; i <= 3; ++i)
    {
        nodes.clear();
        bot.update(Vector3<float>{static_cast<float>(i) + 0.1f, 0.f, 0.f});
    }

    // Displace the bot back to near node 1. The bot should route toward node 1
    // first rather than skipping ahead to node 4.
    nodes.clear();
    bot.update({1.1f, 0.f, 0.f});

    ASSERT_FALSE(nodes.empty());
    EXPECT_LE(nodes.back().x, 3.f);
}

// ---------------------------------------------------------------------------
// Callback wiring
// ---------------------------------------------------------------------------

TEST(WalkBotTests, ReplacingPathCallbackTakesEffect)
{
    auto mesh = make_linear_mesh(4);
    WalkBot bot(mesh, 0.5f);
    bot.set_target({3.f, 0.f, 0.f});

    int first_cb_calls = 0;
    int second_cb_calls = 0;

    bot.on_path([&](const Vector3<float>&) { ++first_cb_calls; });
    bot.update({0.f, 0.f, 0.f});

    bot.on_path([&](const Vector3<float>&) { ++second_cb_calls; });
    bot.update({0.f, 0.f, 0.f});

    EXPECT_EQ(first_cb_calls, 1);
    EXPECT_EQ(second_cb_calls, 1);
}

TEST(WalkBotTests, ReplacingStatusCallbackTakesEffect)
{
    auto mesh = make_linear_mesh(4);
    WalkBot bot(mesh, 0.5f);
    bot.set_target({3.f, 0.f, 0.f});

    std::vector<Vector3<float>> nodes;
    bot.on_path(make_node_recorder(nodes));

    int cb1 = 0, cb2 = 0;
    bot.on_status([&](WalkBotStatus) { ++cb1; });
    bot.update({0.f, 0.f, 0.f});

    bot.on_status([&](WalkBotStatus) { ++cb2; });
    bot.update({0.f, 0.f, 0.f});

    EXPECT_EQ(cb1, 1);
    EXPECT_EQ(cb2, 1);
}

// ---------------------------------------------------------------------------
// Full walk simulation — bot traverses a linear mesh step by step
// ---------------------------------------------------------------------------

// Simulates one game-loop tick and immediately "teleports" the bot to the
// suggested node so the next tick starts from there.
struct WalkSimulator
{
    WalkBot bot;
    Vector3<float> position;
    std::vector<Vector3<float>> visited_nodes;
    std::vector<WalkBotStatus> status_history;
    bool finished{false};

    WalkSimulator(const std::shared_ptr<NavigationMesh>& mesh,
                  const Vector3<float>& start,
                  const Vector3<float>& goal,
                  float threshold)
        : position(start)
    {
        bot = WalkBot(mesh, threshold);
        bot.set_target(goal);
        bot.on_path([this](const Vector3<float>& next) {
            visited_nodes.push_back(next);
            position = next; // teleport to the suggested node
        });
        bot.on_status([this](WalkBotStatus s) {
            status_history.push_back(s);
            if (s == WalkBotStatus::FINISHED)
                finished = true;
        });
    }

    void run(int max_ticks = 100)
    {
        for (int tick = 0; tick < max_ticks && !finished; ++tick)
            bot.update(position);
    }
};

TEST(WalkBotSimulation, BotReachesTargetOnLinearMesh)
{
    auto mesh = make_linear_mesh(5);
    WalkSimulator sim(mesh, {0.f, 0.f, 0.f}, {4.f, 0.f, 0.f}, 0.5f);
    sim.run(50);

    EXPECT_TRUE(sim.finished);
}

TEST(WalkBotSimulation, StatusTransitionSequenceIsCorrect)
{
    // Expect: one or more PATHING updates, then exactly FINISHED at the end.
    auto mesh = make_linear_mesh(4);
    WalkSimulator sim(mesh, {0.f, 0.f, 0.f}, {3.f, 0.f, 0.f}, 0.5f);
    sim.run(50);

    ASSERT_TRUE(sim.finished);
    ASSERT_FALSE(sim.status_history.empty());

    // All entries before the last must be PATHING
    for (std::size_t i = 0; i + 1 < sim.status_history.size(); ++i)
        EXPECT_EQ(sim.status_history[i], WalkBotStatus::PATHING);

    EXPECT_EQ(sim.status_history.back(), WalkBotStatus::FINISHED);
}

TEST(WalkBotSimulation, BotVisitsNodesInForwardOrder)
{
    auto mesh = make_linear_mesh(5);
    WalkSimulator sim(mesh, {0.f, 0.f, 0.f}, {4.f, 0.f, 0.f}, 0.5f);
    sim.run(50);

    ASSERT_FALSE(sim.visited_nodes.empty());

    // Verify that x-coordinates are non-decreasing (forward progress only).
    for (std::size_t i = 1; i < sim.visited_nodes.size(); ++i)
        EXPECT_GE(sim.visited_nodes[i].x, sim.visited_nodes[i - 1].x - 1e-3f);
}

TEST(WalkBotSimulation, TwoNodePathReachesGoal)
{
    auto mesh = make_linear_mesh(2); // (0,0,0) <-> (1,0,0)
    WalkSimulator sim(mesh, {0.f, 0.f, 0.f}, {1.f, 0.f, 0.f}, 0.5f);
    sim.run(10);

    EXPECT_TRUE(sim.finished);
}

TEST(WalkBotSimulation, LongChainEventuallyFinishes)
{
    constexpr int kLength = 20;
    auto mesh = make_linear_mesh(kLength);
    WalkSimulator sim(mesh,
                      {0.f, 0.f, 0.f},
                      {static_cast<float>(kLength - 1), 0.f, 0.f},
                      0.5f);
    sim.run(200);

    EXPECT_TRUE(sim.finished);
}

TEST(WalkBotSimulation, StartAlreadyAtTargetFinishesImmediately)
{
    auto mesh = make_linear_mesh(3);
    WalkSimulator sim(mesh, {1.f, 0.f, 0.f}, {1.f, 0.f, 0.f}, 0.5f);
    sim.run(5);

    EXPECT_TRUE(sim.finished);
    EXPECT_EQ(sim.status_history.front(), WalkBotStatus::FINISHED);
    EXPECT_EQ(sim.status_history.size(), 1u);
}

TEST(WalkBotSimulation, NoIdleEmittedDuringSuccessfulWalk)
{
    auto mesh = make_linear_mesh(4);
    WalkSimulator sim(mesh, {0.f, 0.f, 0.f}, {3.f, 0.f, 0.f}, 0.5f);
    sim.run(50);

    for (auto s : sim.status_history)
        EXPECT_NE(s, WalkBotStatus::IDLE);
}

// ---------------------------------------------------------------------------
// Walk simulation on a branching mesh
// ---------------------------------------------------------------------------

TEST(WalkBotSimulation, BotNavigatesBranchingMesh)
{
    // Diamond topology:
    //        (1,1,0)
    //       /       \
    // (0,0,0)       (2,0,0)
    //       \       /
    //        (1,-1,0)
    auto mesh = std::make_shared<NavigationMesh>();
    const Vector3<float> start{0.f, 0.f, 0.f};
    const Vector3<float> top{1.f, 1.f, 0.f};
    const Vector3<float> bot_node{1.f, -1.f, 0.f};
    const Vector3<float> goal{2.f, 0.f, 0.f};

    mesh->m_vertex_map[start]    = {top, bot_node};
    mesh->m_vertex_map[top]      = {start, goal};
    mesh->m_vertex_map[bot_node] = {start, goal};
    mesh->m_vertex_map[goal]     = {top, bot_node};

    WalkSimulator sim(mesh, start, goal, 0.5f);
    sim.run(20);

    EXPECT_TRUE(sim.finished);
}
