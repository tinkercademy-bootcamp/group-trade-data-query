#include <cstdint>
#include <vector>
class AntiServer {
    AntiServer();
    ~AntiServer();
    /** This stores all the fd_s initialised by the anti server along with the
     * status (free(false)/waiting(true)) */
    std::vector<std::pair<int32_t, bool>> sockets_;
    int epoll_fd_;

    /** Makes a lot of sockets to later send messages on */
    void prepare_sockets(int num_sockets);

    /** Setup an epoll listener which watches all the sockets */
    void setup_listener();
};