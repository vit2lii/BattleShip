set(sources
        src/core/boat.cpp
        src/core/board.cpp
        src/player/player.cpp
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
)

set(test_sources
        src/board_test.cpp
)
