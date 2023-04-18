#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <nlohmann/json.hpp>

class SpiDevEntry {
public:
    std::string device_type;
    int offset_number;
    std::vector<std::vector<std::string>> spi_write;

    void print() const {
        std::cout << "Device Type: " << device_type << std::endl;
        std::cout << "Offset Number: " << offset_number << std::endl;
        std::cout << "SPI Write: " << std::endl;
        for (const auto& group : spi_write) {
            for (const auto& write : group) {
                std::cout << "  " << write << std::endl;
            }
            std::cout << std::endl;
        }
    }
};

class SpiDevRequest {
public:
    SpiDevRequest(const std::string &filename, const int offset)
    : filename(filename), offset(offset) { setRawName(); }

    void raw_to_json() {
        std::ifstream infile(filename);
        std::string line;
        nlohmann::json output;

        output["device_type"] = dev_type;
        output["offset_number"] = offset;

        std::vector<std::vector<std::string>> spi_write_groups;
        std::vector<std::string> current_group;

        while (std::getline(infile, line)) {
            line.erase(std::remove(line.begin(), line.end(), '\r'), line.end());

            if (line.empty()) {
                if (!current_group.empty()) {
                    spi_write_groups.push_back(current_group);
                    current_group.clear();
                }
            } else if (line.find("spi_write:") != std::string::npos) {
                current_group.push_back(line);
            }
        }

        if (!current_group.empty()) {
            spi_write_groups.push_back(current_group);
        }

        output["spi_write"] = spi_write_groups;

        std::ofstream outfile(raw_filename + ".json");
        outfile << output.dump(4);
    }

    SpiDevEntry parse_json_file() {
        std::ifstream infile(raw_filename + ".json");
        nlohmann::json input;

        infile >> input;

        devEntry.device_type = input["device_type"];
        devEntry.offset_number = input["offset_number"];

        for (const auto &group : input["spi_write"]) {
            std::vector<std::string> current_group;
            for (const auto &write : group) {
                current_group.push_back(write);
            }
            devEntry.spi_write.push_back(current_group);
        }

        return devEntry;
    }

    SpiDevEntry parse_processed_json_file() {
        std::ifstream infile(raw_filename + "_processed" + ".json");
        nlohmann::json input;

        infile >> input;

        devEntry.device_type = input["device_type"];
        devEntry.offset_number = input["offset_number"];

        for (const auto &group : input["spi_write"]) {
            std::vector<std::string> current_group;
            for (const auto &write_pair : group) {
                for (const auto &write : write_pair) {
                    current_group.push_back(write);
                }
            }
            devEntry.spi_write.push_back(current_group);
        }

        return devEntry;
    }
    
    void process_and_save_json() {
        nlohmann::json input, output;
        std::ifstream infile(raw_filename + ".json");
        infile >> input;

        output["device_type"] = input["device_type"];
        output["offset_number"] = input["offset_number"];

        std::vector<std::vector<std::vector<std::string>>> spi_write_groups;
        for (const auto &group : input["spi_write"]) {
            std::vector<std::vector<std::string>> current_group;
            for (const auto &write : group) {
                std::stringstream ss(write.get<std::string>());
                std::string temp;
                std::vector<std::string> tokens;

                while (ss >> temp) {
                    tokens.push_back(temp);
                }

                int data_size = std::stoi(tokens[3].substr(0, tokens[3].find(':')));
                std::string buffer = tokens[5];

                for (size_t i = 6; i < tokens.size(); i++) {
                    buffer += " " + tokens[i];
                }

                std::string new_write_1 = "write-" + output["device_type"].get<std::string>() + "device-" + std::to_string(output["offset_number"].get<int>()) + "-buffer-" + buffer;
                std::string new_write_2 = "write-" + output["device_type"].get<std::string>() + "device-" + std::to_string(output["offset_number"].get<int>()) + "-data_size-" + std::to_string(data_size);

                std::vector<std::string> write_pair = {new_write_1, new_write_2};
                current_group.push_back(write_pair);
            }
            spi_write_groups.push_back(current_group);
        }

        output["spi_write"] = spi_write_groups;

        std::ofstream outfile(raw_filename + "_processed" + ".json");
        outfile << output.dump(4);
    }

    SpiDevEntry getDevEntry() const {
        return devEntry;
    }

private:
    std::string filename;
    std::string raw_filename;

    const std::string dev_type = "spi";
    int offset;      
    SpiDevEntry devEntry;

    void setRawName() {
        size_t pos = filename.find_last_of(".");

        if (pos != std::string::npos) {
            raw_filename = filename.substr(0, pos);
        }

        else
            raw_filename = filename;
    }
};

int main() {
    SpiDevRequest spi("SPI_A.txt", 0);
    spi.raw_to_json();
    spi.parse_json_file();
    spi.getDevEntry().print();
    spi.process_and_save_json();
    spi.parse_processed_json_file();
    spi.getDevEntry().print();

    return 0;
}
