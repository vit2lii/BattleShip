#include <boost/beast/http.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <gtest/gtest.h>

#include <functional>
#include <optional>
#include <sstream>
#include <string>
#include <vector>

#include "server/game_store.hpp"
#include "server/http_router.hpp"

namespace server::tests
{
  namespace http = boost::beast::http;
  namespace pt = boost::property_tree;

  namespace
  {
    void expectRuntimeError(const std::function<void()>& fn, std::string_view expected)
    {
      try
      {
        fn();
        FAIL() << "Expected std::runtime_error with message: " << expected;
      }
      catch (const std::runtime_error& e)
      {
        EXPECT_EQ(e.what(), expected);
      }
      catch (...)
      {
        FAIL() << "Expected std::runtime_error";
      }
    }

    pt::ptree parseJson(const std::string& body)
    {
      std::istringstream iss(body);
      pt::ptree tree;
      pt::read_json(iss, tree);
      return tree;
    }

    http::request<http::string_body> buildRequest(http::verb method,
                                                   const std::string& target,
                                                   const std::string& body = "",
                                                   const std::optional<std::string>& auth = std::nullopt)
    {
      http::request<http::string_body> req{ method, target, 11 };
      req.set(http::field::host, "localhost");
      req.set(http::field::user_agent, "test");

      if (auth.has_value())
      {
        req.set(http::field::authorization, *auth);
      }
      if (!body.empty())
      {
        req.set(http::field::content_type, "application/json");
        req.body() = body;
      }

      req.prepare_payload();
      return req;
    }

    std::string bearer(const std::string& token)
    {
      return "Bearer " + token;
    }
  }  // namespace

  class GameStoreTest : public ::testing::Test
  {
   protected:
    GameStore store;
  };

  TEST_F(GameStoreTest, CreateGameReturnsExpectedInitialValues)
  {
    const auto out = store.createGame();
    EXPECT_FALSE(out.gameId.empty());
    EXPECT_EQ(out.playerId, 1);
    EXPECT_EQ(out.status, GameStatus::WaitingForPlayers);
    EXPECT_EQ(out.playerToken.size(), 27U);
    EXPECT_NE(out.playerToken.find('-'), std::string::npos);
  }

  TEST_F(GameStoreTest, JoinGameSucceedsAndSetsPlacingStatus)
  {
    const auto created = store.createGame();

    const auto joined = store.joinGame(created.gameId);
    EXPECT_EQ(joined.gameId, created.gameId);
    EXPECT_EQ(joined.playerId, 2);
    EXPECT_EQ(joined.status, GameStatus::Placing);
    EXPECT_FALSE(joined.playerToken.empty());
  }

  TEST_F(GameStoreTest, JoinGameTwiceThrowsExpectedError)
  {
    const auto created = store.createGame();
    (void)store.joinGame(created.gameId);

    expectRuntimeError([&]() { (void)store.joinGame(created.gameId); }, "Game already has 2 players.");
  }

  TEST_F(GameStoreTest, JoinUnknownGameThrowsExpectedError)
  {
    expectRuntimeError([&]() { (void)store.joinGame("missing-game-id"); }, "Game not found.");
  }

  TEST_F(GameStoreTest, AuthenticateHandlesValidAndInvalidInputs)
  {
    const auto created = store.createGame();
    const auto joined = store.joinGame(created.gameId);

    const auto authP1 = store.authenticate(created.gameId, bearer(created.playerToken));
    EXPECT_EQ(authP1.playerIndex, 0);
    EXPECT_EQ(authP1.token, created.playerToken);

    const auto authP2 = store.authenticate(created.gameId, bearer(joined.playerToken));
    EXPECT_EQ(authP2.playerIndex, 1);
    EXPECT_EQ(authP2.token, joined.playerToken);

    const auto authInvalidToken = store.authenticate(created.gameId, "Bearer wrong-token");
    EXPECT_EQ(authInvalidToken.playerIndex, -1);
    EXPECT_EQ(authInvalidToken.token, "wrong-token");

    const auto authMissingPrefix = store.authenticate(created.gameId, created.playerToken);
    EXPECT_EQ(authMissingPrefix.playerIndex, -1);
    EXPECT_TRUE(authMissingPrefix.token.empty());

    const auto authMissingGame = store.authenticate("missing-game-id", bearer(created.playerToken));
    EXPECT_EQ(authMissingGame.playerIndex, -1);
    EXPECT_TRUE(authMissingGame.token.empty());
  }

  TEST_F(GameStoreTest, ReadyUpTransitionsPlacingToInProgress)
  {
    const auto created = store.createGame();
    (void)store.joinGame(created.gameId);

    EXPECT_EQ(store.readyUp(created.gameId, 0), GameStatus::Placing);
    EXPECT_EQ(store.readyUp(created.gameId, 1), GameStatus::InProgress);

    const auto view = store.getGameView(created.gameId);
    ASSERT_TRUE(view.has_value());
    EXPECT_EQ(view->status, GameStatus::InProgress);
    EXPECT_EQ(view->turn, 0);
    EXPECT_TRUE(view->ready[0]);
    EXPECT_TRUE(view->ready[1]);
  }

  TEST_F(GameStoreTest, ShootBeforeInProgressThrowsExpectedError)
  {
    const auto created = store.createGame();
    (void)store.joinGame(created.gameId);

    expectRuntimeError(
        [&]() { (void)store.shoot(created.gameId, 0, battleship::Coordinate{ 0, 0 }); }, "Game not in progress.");
  }

  TEST_F(GameStoreTest, ShootOutOfTurnThrowsExpectedError)
  {
    const auto created = store.createGame();
    (void)store.joinGame(created.gameId);
    (void)store.readyUp(created.gameId, 0);
    (void)store.readyUp(created.gameId, 1);

    expectRuntimeError([&]() { (void)store.shoot(created.gameId, 1, battleship::Coordinate{ 0, 0 }); }, "Not your turn.");
  }

  TEST_F(GameStoreTest, GetGameViewReturnsNulloptForUnknownGame)
  {
    EXPECT_FALSE(store.getGameView("missing-game-id").has_value());
  }

  TEST_F(GameStoreTest, GetGameViewReturnsExpectedStateForExistingGame)
  {
    const auto created = store.createGame();
    (void)store.joinGame(created.gameId);
    (void)store.readyUp(created.gameId, 0);

    const auto view = store.getGameView(created.gameId);
    ASSERT_TRUE(view.has_value());
    EXPECT_EQ(view->status, GameStatus::Placing);
    EXPECT_EQ(view->turn, 0);
    EXPECT_TRUE(view->ready[0]);
    EXPECT_FALSE(view->ready[1]);
  }

  TEST_F(GameStoreTest, FirstLegalShotCanFinishGameWhenEnemyHasNoBoats)
  {
    const auto created = store.createGame();
    (void)store.joinGame(created.gameId);
    (void)store.readyUp(created.gameId, 0);
    (void)store.readyUp(created.gameId, 1);

    const auto out = store.shoot(created.gameId, 0, battleship::Coordinate{ 0, 0 });
    EXPECT_EQ(out.result, "OK");
    EXPECT_EQ(out.nextTurnPlayerId, 1);
    EXPECT_EQ(out.status, GameStatus::Finished);

    const auto view = store.getGameView(created.gameId);
    ASSERT_TRUE(view.has_value());
    EXPECT_EQ(view->status, GameStatus::Finished);
    EXPECT_EQ(view->turn, 0);
  }

  class HttpRouterTest : public ::testing::Test
  {
   protected:
    GameStore store;

    http::response<http::string_body> send(http::verb method,
                                           const std::string& target,
                                           const std::string& body = "",
                                           const std::optional<std::string>& auth = std::nullopt)
    {
      return handle_request(store, buildRequest(method, target, body, auth));
    }
  };

  TEST_F(HttpRouterTest, PostGamesReturnsCreatePayload)
  {
    const auto res = send(http::verb::post, "/games");
    ASSERT_EQ(res.result(), http::status::ok);

    const auto json = parseJson(res.body());
    EXPECT_FALSE(json.get<std::string>("gameId").empty());
    EXPECT_EQ(json.get<int>("playerId"), 1);
    EXPECT_EQ(json.get<std::string>("status"), "waiting_for_players");
    EXPECT_FALSE(json.get<std::string>("playerToken").empty());
  }

  TEST_F(HttpRouterTest, JoinEndpointSucceedsThenConflictsOnSecondJoin)
  {
    const auto createRes = send(http::verb::post, "/games");
    ASSERT_EQ(createRes.result(), http::status::ok);
    const auto created = parseJson(createRes.body());
    const std::string gameId = created.get<std::string>("gameId");

    const auto joinRes1 = send(http::verb::post, "/games/" + gameId + "/join");
    ASSERT_EQ(joinRes1.result(), http::status::ok);
    const auto joined = parseJson(joinRes1.body());
    EXPECT_EQ(joined.get<int>("playerId"), 2);
    EXPECT_EQ(joined.get<std::string>("status"), "placing");

    const auto joinRes2 = send(http::verb::post, "/games/" + gameId + "/join");
    EXPECT_EQ(joinRes2.result(), http::status::conflict);
    EXPECT_EQ(joinRes2.body(), "Game already has 2 players.");
  }

  TEST_F(HttpRouterTest, JoinUnknownGameReturnsNotFound)
  {
    const auto res = send(http::verb::post, "/games/missing-game-id/join");
    EXPECT_EQ(res.result(), http::status::not_found);
    EXPECT_EQ(res.body(), "Game not found.");
  }

  TEST_F(HttpRouterTest, ProtectedRouteWithoutAuthorizationReturnsUnauthorized)
  {
    const auto createRes = send(http::verb::post, "/games");
    ASSERT_EQ(createRes.result(), http::status::ok);
    const std::string gameId = parseJson(createRes.body()).get<std::string>("gameId");

    const auto res = send(http::verb::get, "/games/" + gameId);
    EXPECT_EQ(res.result(), http::status::unauthorized);
    EXPECT_EQ(res.body(), "Unauthorized");
  }

  TEST_F(HttpRouterTest, ReadyWithoutAuthorizationReturnsUnauthorized)
  {
    const auto createRes = send(http::verb::post, "/games");
    ASSERT_EQ(createRes.result(), http::status::ok);
    const std::string gameId = parseJson(createRes.body()).get<std::string>("gameId");

    const auto res = send(http::verb::post, "/games/" + gameId + "/ready");
    EXPECT_EQ(res.result(), http::status::unauthorized);
    EXPECT_EQ(res.body(), "Unauthorized");
  }

  TEST_F(HttpRouterTest, PlaceWithMalformedJsonReturnsBadRequest)
  {
    const auto created = store.createGame();
    (void)store.joinGame(created.gameId);

    const auto res = send(http::verb::post, "/games/" + created.gameId + "/place", "{invalid", bearer(created.playerToken));
    EXPECT_EQ(res.result(), http::status::bad_request);
    EXPECT_EQ(res.body(), "Invalid JSON");
  }

  TEST_F(HttpRouterTest, ShootWithMalformedJsonReturnsBadRequest)
  {
    const auto created = store.createGame();
    (void)store.joinGame(created.gameId);

    const auto res = send(http::verb::post, "/games/" + created.gameId + "/shoot", "{invalid", bearer(created.playerToken));
    EXPECT_EQ(res.result(), http::status::bad_request);
    EXPECT_EQ(res.body(), "Invalid JSON");
  }

  TEST_F(HttpRouterTest, PlaceWithMissingFieldsReturnsBadRequest)
  {
    const auto created = store.createGame();
    (void)store.joinGame(created.gameId);

    const auto res = send(http::verb::post,
                          "/games/" + created.gameId + "/place",
                          R"({"type":"DESTROYER"})",
                          bearer(created.playerToken));
    EXPECT_EQ(res.result(), http::status::bad_request);
    EXPECT_EQ(res.body(), "Missing fields: type/start/orientation");
  }

  TEST_F(HttpRouterTest, PlaceWithInvalidBoatTypeReturnsBadRequest)
  {
    const auto created = store.createGame();
    (void)store.joinGame(created.gameId);

    const auto res = send(http::verb::post,
                          "/games/" + created.gameId + "/place",
                          R"({"type":"INVALID","start":"A1","orientation":"E"})",
                          bearer(created.playerToken));
    EXPECT_EQ(res.result(), http::status::bad_request);
    EXPECT_EQ(res.body(), "Invalid boat type.");
  }

  TEST_F(HttpRouterTest, PlaceWithInvalidOrientationReturnsBadRequest)
  {
    const auto created = store.createGame();
    (void)store.joinGame(created.gameId);

    const auto res = send(http::verb::post,
                          "/games/" + created.gameId + "/place",
                          R"({"type":"DESTROYER","start":"A1","orientation":"Q"})",
                          bearer(created.playerToken));
    EXPECT_EQ(res.result(), http::status::bad_request);
    EXPECT_EQ(res.body(), "Invalid orientation (use N/S/E/W). ");
  }

  TEST_F(HttpRouterTest, PlaceWithInvalidCoordinateReturnsBadRequest)
  {
    const auto created = store.createGame();
    (void)store.joinGame(created.gameId);

    const auto res = send(http::verb::post,
                          "/games/" + created.gameId + "/place",
                          R"({"type":"DESTROYER","start":"Z1","orientation":"E"})",
                          bearer(created.playerToken));
    EXPECT_EQ(res.result(), http::status::bad_request);
    EXPECT_EQ(res.body(), "Invalid row character");
  }

  TEST_F(HttpRouterTest, ReadyEndpointReportsExpectedTransitions)
  {
    const auto createRes = send(http::verb::post, "/games");
    ASSERT_EQ(createRes.result(), http::status::ok);
    const auto created = parseJson(createRes.body());
    const std::string gameId = created.get<std::string>("gameId");
    const std::string p1Token = created.get<std::string>("playerToken");

    const auto joinRes = send(http::verb::post, "/games/" + gameId + "/join");
    ASSERT_EQ(joinRes.result(), http::status::ok);
    const std::string p2Token = parseJson(joinRes.body()).get<std::string>("playerToken");

    const auto readyP1 = send(http::verb::post, "/games/" + gameId + "/ready", "", bearer(p1Token));
    ASSERT_EQ(readyP1.result(), http::status::ok);
    EXPECT_EQ(parseJson(readyP1.body()).get<std::string>("status"), "placing");

    const auto readyP2 = send(http::verb::post, "/games/" + gameId + "/ready", "", bearer(p2Token));
    ASSERT_EQ(readyP2.result(), http::status::ok);
    EXPECT_EQ(parseJson(readyP2.body()).get<std::string>("status"), "in_progress");
  }

  TEST_F(HttpRouterTest, ShootBeforeGameInProgressReturnsConflict)
  {
    const auto createRes = send(http::verb::post, "/games");
    ASSERT_EQ(createRes.result(), http::status::ok);
    const auto created = parseJson(createRes.body());
    const std::string gameId = created.get<std::string>("gameId");
    const std::string p1Token = created.get<std::string>("playerToken");
    const auto joinRes = send(http::verb::post, "/games/" + gameId + "/join");
    ASSERT_EQ(joinRes.result(), http::status::ok);

    const auto shootRes =
        send(http::verb::post, "/games/" + gameId + "/shoot", R"({"target":"A1"})", bearer(p1Token));
    EXPECT_EQ(shootRes.result(), http::status::conflict);
    EXPECT_EQ(shootRes.body(), "Game not in progress.");
  }

  TEST_F(HttpRouterTest, ShootWithMissingTargetReturnsBadRequest)
  {
    const auto created = store.createGame();
    (void)store.joinGame(created.gameId);

    const auto res = send(http::verb::post, "/games/" + created.gameId + "/shoot", "{}", bearer(created.playerToken));
    EXPECT_EQ(res.result(), http::status::bad_request);
    EXPECT_EQ(res.body(), "Missing field: target");
  }

  TEST_F(HttpRouterTest, ShootWithInvalidCoordinateReturnsBadRequest)
  {
    const auto created = store.createGame();
    (void)store.joinGame(created.gameId);

    const auto res =
        send(http::verb::post, "/games/" + created.gameId + "/shoot", R"({"target":"Z1"})", bearer(created.playerToken));
    EXPECT_EQ(res.result(), http::status::bad_request);
    EXPECT_EQ(res.body(), "Invalid row character");
  }

  TEST_F(HttpRouterTest, ShootOutOfTurnReturnsConflict)
  {
    const auto createRes = send(http::verb::post, "/games");
    ASSERT_EQ(createRes.result(), http::status::ok);
    const auto created = parseJson(createRes.body());
    const std::string gameId = created.get<std::string>("gameId");
    const std::string p1Token = created.get<std::string>("playerToken");

    const auto joinRes = send(http::verb::post, "/games/" + gameId + "/join");
    ASSERT_EQ(joinRes.result(), http::status::ok);
    const std::string p2Token = parseJson(joinRes.body()).get<std::string>("playerToken");

    ASSERT_EQ(send(http::verb::post, "/games/" + gameId + "/ready", "", bearer(p1Token)).result(), http::status::ok);
    ASSERT_EQ(send(http::verb::post, "/games/" + gameId + "/ready", "", bearer(p2Token)).result(), http::status::ok);

    const auto res = send(http::verb::post, "/games/" + gameId + "/shoot", R"({"target":"A1"})", bearer(p2Token));
    EXPECT_EQ(res.result(), http::status::conflict);
    EXPECT_EQ(res.body(), "Not your turn.");
  }

  TEST_F(HttpRouterTest, ShootAlreadyShotReturnsConflict)
  {
    const auto createRes = send(http::verb::post, "/games");
    ASSERT_EQ(createRes.result(), http::status::ok);
    const auto created = parseJson(createRes.body());
    const std::string gameId = created.get<std::string>("gameId");
    const std::string p1Token = created.get<std::string>("playerToken");

    const auto joinRes = send(http::verb::post, "/games/" + gameId + "/join");
    ASSERT_EQ(joinRes.result(), http::status::ok);
    const std::string p2Token = parseJson(joinRes.body()).get<std::string>("playerToken");

    ASSERT_EQ(send(http::verb::post,
                   "/games/" + gameId + "/place",
                   R"({"type":"DESTROYER","start":"J10","orientation":"W"})",
                   bearer(p1Token))
                  .result(),
              http::status::ok);
    ASSERT_EQ(send(http::verb::post,
                   "/games/" + gameId + "/place",
                   R"({"type":"DESTROYER","start":"J10","orientation":"W"})",
                   bearer(p2Token))
                  .result(),
              http::status::ok);

    ASSERT_EQ(send(http::verb::post, "/games/" + gameId + "/ready", "", bearer(p1Token)).result(), http::status::ok);
    ASSERT_EQ(send(http::verb::post, "/games/" + gameId + "/ready", "", bearer(p2Token)).result(), http::status::ok);

    ASSERT_EQ(send(http::verb::post, "/games/" + gameId + "/shoot", R"({"target":"A1"})", bearer(p1Token)).result(),
              http::status::ok);
    ASSERT_EQ(send(http::verb::post, "/games/" + gameId + "/shoot", R"({"target":"A2"})", bearer(p2Token)).result(),
              http::status::ok);

    const auto res = send(http::verb::post, "/games/" + gameId + "/shoot", R"({"target":"A1"})", bearer(p1Token));
    EXPECT_EQ(res.result(), http::status::conflict);
    EXPECT_EQ(res.body(), "Cell has already been shot.");
  }

  TEST_F(HttpRouterTest, UnknownRouteReturnsNotFound)
  {
    const auto res = send(http::verb::get, "/unknown");
    EXPECT_EQ(res.result(), http::status::not_found);
    EXPECT_EQ(res.body(), "Not found");
  }

  TEST_F(HttpRouterTest, GetGameWithAuthorizationReturnsExpectedEnvelope)
  {
    const auto createRes = send(http::verb::post, "/games");
    ASSERT_EQ(createRes.result(), http::status::ok);
    const auto created = parseJson(createRes.body());
    const std::string gameId = created.get<std::string>("gameId");
    const std::string p1Token = created.get<std::string>("playerToken");
    const auto joinRes = send(http::verb::post, "/games/" + gameId + "/join");
    ASSERT_EQ(joinRes.result(), http::status::ok);

    const auto getRes = send(http::verb::get, "/games/" + gameId, "", bearer(p1Token));
    ASSERT_EQ(getRes.result(), http::status::ok);

    const auto json = parseJson(getRes.body());
    EXPECT_EQ(json.get<std::string>("gameId"), gameId);
    EXPECT_EQ(json.get<std::string>("status"), "placing");
    EXPECT_EQ(json.get<int>("turnPlayerId"), 1);
    EXPECT_EQ(json.get<int>("you.playerId"), 1);
    EXPECT_FALSE(json.get<bool>("you.ready"));
    EXPECT_EQ(json.get<int>("yourBoard.width"), 10);
    EXPECT_EQ(json.get<int>("enemyBoard.width"), 10);
    EXPECT_EQ(json.get_child("yourBoard.cells").size(), 100U);
    EXPECT_EQ(json.get_child("enemyBoard.cells").size(), 100U);
  }

  TEST_F(HttpRouterTest, GetGameSupportsQueryString)
  {
    const auto createRes = send(http::verb::post, "/games");
    ASSERT_EQ(createRes.result(), http::status::ok);
    const auto created = parseJson(createRes.body());
    const std::string gameId = created.get<std::string>("gameId");
    const std::string p1Token = created.get<std::string>("playerToken");
    const auto joinRes = send(http::verb::post, "/games/" + gameId + "/join");
    ASSERT_EQ(joinRes.result(), http::status::ok);

    const auto res = send(http::verb::get, "/games/" + gameId + "?x=1", "", bearer(p1Token));
    ASSERT_EQ(res.result(), http::status::ok);
    EXPECT_EQ(parseJson(res.body()).get<std::string>("gameId"), gameId);
  }

  TEST_F(HttpRouterTest, GetGameMasksEnemyOccupiedCells)
  {
    const auto createRes = send(http::verb::post, "/games");
    ASSERT_EQ(createRes.result(), http::status::ok);
    const auto created = parseJson(createRes.body());
    const std::string gameId = created.get<std::string>("gameId");
    const std::string p1Token = created.get<std::string>("playerToken");

    const auto joinRes = send(http::verb::post, "/games/" + gameId + "/join");
    ASSERT_EQ(joinRes.result(), http::status::ok);
    const std::string p2Token = parseJson(joinRes.body()).get<std::string>("playerToken");

    ASSERT_EQ(send(http::verb::post,
                   "/games/" + gameId + "/place",
                   R"({"type":"DESTROYER","start":"J10","orientation":"W"})",
                   bearer(p2Token))
                  .result(),
              http::status::ok);

    const auto res = send(http::verb::get, "/games/" + gameId, "", bearer(p1Token));
    ASSERT_EQ(res.result(), http::status::ok);

    const auto json = parseJson(res.body());
    std::vector<std::string> cells;
    for (const auto& node : json.get_child("enemyBoard.cells"))
    {
      cells.push_back(node.second.get_value<std::string>());
    }

    ASSERT_EQ(cells.size(), 100U);
    EXPECT_EQ(cells[98], "empty");
    EXPECT_EQ(cells[99], "empty");
  }

}  // namespace server::tests
