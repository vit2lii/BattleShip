set(sources
        src/core/boat.cpp
        src/core/board.cpp
        src/player/player.cpp
        src/server/game_store.cpp
        src/server/http_router.cpp
        src/server/http_server.cpp
)

set(exe_sources
        src/main.cpp
        ${sources}
)

set(headers
        include/project/core/boat.hpp
        include/project/core/board.hpp
        include/project/core/coordinate.hpp
        include/project/core/placement.hpp
        include/project/core/structure.hpp
        include/project/core/boardPrinter.hpp
        include/project/player/player.hpp
        include/project/exceptions/exceptions.hpp
        include/project/core/cell.hpp
        include/server/game_store.hpp
        include/server/game_types.hpp
        include/server/http_router.hpp
        include/server/http_server.hpp
)

set(test_sources
        src/board_test.cpp
        server/test_server.cpp
)
