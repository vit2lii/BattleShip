#include "server/http_router.hpp"

#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>

#include <cctype>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

namespace server
{
  namespace http = boost::beast::http;
  namespace pt = boost::property_tree;

  namespace
  {
    http::response<http::string_body> make_response(http::status status,
                                                    std::string body,
                                                    unsigned version,
                                                    bool keep_alive,
                                                    std::string_view content_type = "text/plain")
    {
      http::response<http::string_body> res{ status, version };
      res.set(http::field::server, "BattleShip");
      res.set(http::field::content_type, content_type);
      res.keep_alive(keep_alive);
      res.body() = std::move(body);
      res.prepare_payload();
      return res;
    }

    http::response<http::string_body> make_json_response(http::status status,
                                                         const pt::ptree& body,
                                                         unsigned version,
                                                         bool keep_alive)
    {
      std::ostringstream oss;
      pt::write_json(oss, body, false);
      return make_response(status, oss.str(), version, keep_alive, "application/json");
    }

    std::vector<std::string> split_path(std::string path)
    {
      const std::size_t query_pos = path.find('?');
      if (query_pos != std::string::npos)
      {
        path.erase(query_pos);
      }

      std::vector<std::string> parts;
      std::stringstream ss(path);
      std::string part;
      while (std::getline(ss, part, '/'))
      {
        if (!part.empty())
        {
          parts.push_back(part);
        }
      }
      return parts;
    }

    pt::ptree parse_json_or_throw(const std::string& body)
    {
      std::istringstream iss(body);
      pt::ptree tree;
      try
      {
        pt::read_json(iss, tree);
      }
      catch (const pt::json_parser_error&)
      {
        throw std::invalid_argument("Invalid JSON");
      }
      return tree;
    }

    battleship::BoatType parse_boat_type(const std::string& raw)
    {
      if (raw == "CARRIER")
      {
        return battleship::BoatType::CARRIER;
      }
      if (raw == "BATTLESHIP")
      {
        return battleship::BoatType::BATTLESHIP;
      }
      if (raw == "CRUISER")
      {
        return battleship::BoatType::CRUISER;
      }
      if (raw == "SUBMARINE")
      {
        return battleship::BoatType::SUBMARINE;
      }
      if (raw == "DESTROYER")
      {
        return battleship::BoatType::DESTROYER;
      }

      throw std::invalid_argument("Invalid boat type.");
    }

    battleship::Orientation parse_orientation(const std::string& raw)
    {
      if (raw.size() != 1U)
      {
        throw std::invalid_argument("Invalid orientation (use N/S/E/W). ");
      }

      switch (std::toupper(static_cast<unsigned char>(raw[0])))
      {
        case 'N': return battleship::Orientation::NORTH;
        case 'S': return battleship::Orientation::SOUTH;
        case 'E': return battleship::Orientation::EAST;
        case 'W': return battleship::Orientation::WEST;
        default: throw std::invalid_argument("Invalid orientation (use N/S/E/W). ");
      }
    }

    std::string cell_state_to_string(battleship::CellState state)
    {
      switch (state)
      {
        case battleship::CellState::EMPTY: return "empty";
        case battleship::CellState::OCCUPIED: return "occupied";
        case battleship::CellState::HIT: return "hit";
        case battleship::CellState::MISS: return "miss";
      }
      return "unknown";
    }

    pt::ptree make_board_json(const GameView& view, int board_index, bool reveal_occupied)
    {
      pt::ptree board;
      board.put("width", static_cast<int>(battleship::BOARD_SIZE));
      board.put("height", static_cast<int>(battleship::BOARD_SIZE));

      pt::ptree cells;
      for (int r = 0; r < static_cast<int>(battleship::BOARD_SIZE); ++r)
      {
        for (int c = 0; c < static_cast<int>(battleship::BOARD_SIZE); ++c)
        {
          auto state = view.boards[board_index].cells[static_cast<std::size_t>(r)][static_cast<std::size_t>(c)];
          if (!reveal_occupied && state == battleship::CellState::OCCUPIED)
          {
            state = battleship::CellState::EMPTY;
          }

          pt::ptree cell_value;
          cell_value.put("", cell_state_to_string(state));
          cells.push_back({ "", cell_value });
        }
      }

      board.add_child("cells", cells);
      return board;
    }

    AuthContext authenticate_request(const GameStore& store,
                                     const std::string& game_id,
                                     const http::request<http::string_body>& req)
    {
      const auto auth_it = req.find(http::field::authorization);
      if (auth_it == req.end())
      {
        return AuthContext{};
      }
      return store.authenticate(game_id, std::string(auth_it->value()));
    }
  }  // namespace

  http::response<http::string_body> handle_request(GameStore& store, http::request<http::string_body> req)
  {
    const auto version = req.version();
    const bool keep_alive = req.keep_alive();
    const auto parts = split_path(std::string(req.target()));

    if (req.method() == http::verb::post && parts.size() == 1 && parts[0] == "games")
    {
      const auto created = store.createGame();
      pt::ptree body;
      body.put("gameId", created.gameId);
      body.put("playerId", created.playerId);
      body.put("playerToken", created.playerToken);
      body.put("status", to_cstr(created.status));
      return make_json_response(http::status::ok, body, version, keep_alive);
    }

    if (parts.size() < 2 || parts[0] != "games")
    {
      return make_response(http::status::not_found, "Not found", version, keep_alive);
    }

    const std::string& game_id = parts[1];

    if (req.method() == http::verb::post && parts.size() == 3 && parts[2] == "join")
    {
      try
      {
        const auto joined = store.joinGame(game_id);
        pt::ptree body;
        body.put("gameId", joined.gameId);
        body.put("playerId", joined.playerId);
        body.put("playerToken", joined.playerToken);
        body.put("status", to_cstr(joined.status));
        return make_json_response(http::status::ok, body, version, keep_alive);
      }
      catch (const std::runtime_error& e)
      {
        const std::string message = e.what();
        if (message == "Game already has 2 players.")
        {
          return make_response(http::status::conflict, message, version, keep_alive);
        }
        if (message == "Game not found.")
        {
          return make_response(http::status::not_found, message, version, keep_alive);
        }
        return make_response(http::status::bad_request, message, version, keep_alive);
      }
    }

    const AuthContext auth = authenticate_request(store, game_id, req);
    if (auth.playerIndex < 0)
    {
      return make_response(http::status::unauthorized, "Unauthorized", version, keep_alive);
    }

    if (req.method() == http::verb::get && parts.size() == 2)
    {
      const auto view = store.getGameView(game_id);
      if (!view.has_value())
      {
        return make_response(http::status::not_found, "Game not found.", version, keep_alive);
      }

      pt::ptree body;
      body.put("gameId", game_id);
      body.put("status", to_cstr(view->status));
      body.put("turnPlayerId", view->turn + 1);
      body.put("you.playerId", auth.playerIndex + 1);
      body.put("you.ready", view->ready[auth.playerIndex]);
      body.add_child("yourBoard", make_board_json(*view, auth.playerIndex, true));
      body.add_child("enemyBoard", make_board_json(*view, 1 - auth.playerIndex, false));
      return make_json_response(http::status::ok, body, version, keep_alive);
    }

    if (req.method() == http::verb::post && parts.size() == 3 && parts[2] == "place")
    {
      try
      {
        const pt::ptree payload = parse_json_or_throw(req.body());

        const auto type = payload.get_optional<std::string>("type");
        const auto start = payload.get_optional<std::string>("start");
        const auto orientation = payload.get_optional<std::string>("orientation");
        if (!type.has_value() || !start.has_value() || !orientation.has_value())
        {
          return make_response(http::status::bad_request, "Missing fields: type/start/orientation", version, keep_alive);
        }

        const auto boat_type = parse_boat_type(*type);
        const auto start_coord = battleship::Coordinate::parseFromString(*start);
        const auto orient = parse_orientation(*orientation);

        store.placeShip(game_id, auth.playerIndex, boat_type, start_coord, orient);

        pt::ptree body;
        body.put("ok", true);
        return make_json_response(http::status::ok, body, version, keep_alive);
      }
      catch (const std::invalid_argument& e)
      {
        return make_response(http::status::bad_request, e.what(), version, keep_alive);
      }
      catch (const std::runtime_error& e)
      {
        const std::string message = e.what();
        if (message == "Game not found.")
        {
          return make_response(http::status::not_found, message, version, keep_alive);
        }
        return make_response(http::status::bad_request, message, version, keep_alive);
      }
    }

    if (req.method() == http::verb::post && parts.size() == 3 && parts[2] == "ready")
    {
      try
      {
        const auto status = store.readyUp(game_id, auth.playerIndex);

        pt::ptree body;
        body.put("status", to_cstr(status));
        return make_json_response(http::status::ok, body, version, keep_alive);
      }
      catch (const std::runtime_error& e)
      {
        const std::string message = e.what();
        if (message == "Game not found.")
        {
          return make_response(http::status::not_found, message, version, keep_alive);
        }
        return make_response(http::status::bad_request, message, version, keep_alive);
      }
    }

    if (req.method() == http::verb::post && parts.size() == 3 && parts[2] == "shoot")
    {
      try
      {
        const pt::ptree payload = parse_json_or_throw(req.body());
        const auto target = payload.get_optional<std::string>("target");
        if (!target.has_value())
        {
          return make_response(http::status::bad_request, "Missing field: target", version, keep_alive);
        }

        const auto target_coord = battleship::Coordinate::parseFromString(*target);
        const auto out = store.shoot(game_id, auth.playerIndex, target_coord);

        pt::ptree body;
        body.put("result", out.result);
        body.put("nextTurnPlayerId", out.nextTurnPlayerId);
        body.put("status", to_cstr(out.status));
        return make_json_response(http::status::ok, body, version, keep_alive);
      }
      catch (const std::invalid_argument& e)
      {
        return make_response(http::status::bad_request, e.what(), version, keep_alive);
      }
      catch (const std::runtime_error& e)
      {
        const std::string message = e.what();
        if (message == "Game not in progress." || message == "Not your turn." || message == "Cell has already been shot.")
        {
          return make_response(http::status::conflict, message, version, keep_alive);
        }
        if (message == "Game not found.")
        {
          return make_response(http::status::not_found, message, version, keep_alive);
        }
        return make_response(http::status::bad_request, message, version, keep_alive);
      }
    }

    return make_response(http::status::not_found, "Not found", version, keep_alive);
  }

}  // namespace server
