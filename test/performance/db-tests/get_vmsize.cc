#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <cstdint>

#define PAGE_SIZE 4096ULL
#define PAGEMAP_ENTRY 8

struct Region {
    uint64_t start;
    uint64_t end;
};

std::vector<Region> get_memory_regions(pid_t pid) {
    std::vector<Region> regions;
    std::ifstream maps("/proc/" + std::to_string(pid) + "/maps");
    std::string line;

    while (std::getline(maps, line)) {
        std::istringstream iss(line);
        std::string addr_range;
        if (!(iss >> addr_range)) continue;

        auto dash = addr_range.find('-');
        uint64_t start = std::stoull(addr_range.substr(0, dash), nullptr, 16);
        uint64_t end = std::stoull(addr_range.substr(dash + 1), nullptr, 16);
        regions.push_back({start, end});
    }

    return regions;
}

bool is_page_present(int pagemap_fd, uint64_t vaddr) {
    uint64_t offset = (vaddr / PAGE_SIZE) * PAGEMAP_ENTRY;
    if (lseek(pagemap_fd, offset, SEEK_SET) == (off_t)-1) {
        return false;
    }

    uint64_t entry;
    if (read(pagemap_fd, &entry, PAGEMAP_ENTRY) != PAGEMAP_ENTRY) {
        return false;
    }

    return entry & (1ULL << 63); // Present bit
}

uint64_t get_resident_memory(pid_t pid) {
    auto regions = get_memory_regions(pid);
    std::string pagemap_path = "/proc/" + std::to_string(pid) + "/pagemap";
    int fd = open(pagemap_path.c_str(), O_RDONLY);
    if (fd < 0) {
        perror("open pagemap");
        exit(1);
        return 0;
    }

    uint64_t total_present = 0;

    for (const auto& region : regions) {
        for (uint64_t addr = region.start; addr < region.end; addr += PAGE_SIZE) {
            if (is_page_present(fd, addr)) {
                total_present++;
            }
        }
    }

    close(fd);
    return total_present * PAGE_SIZE;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <pid>\n";
        return 1;
    }

    pid_t pid = std::stoi(argv[1]);
    int64_t ind = 0;
    while (true) {
        uint64_t rss_bytes = get_resident_memory(pid);
        std::cout << ind++ << " " << rss_bytes / 1024.0 << '\n';
    }

    // std::cout << "Physical memory allocated (resident): " << rss_bytes / 1024.0 << " KB\n";
    return 0;
}
