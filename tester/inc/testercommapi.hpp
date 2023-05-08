/**
 * @file testercommapi.hpp
 * @brief Communication API for tester to provide functionality of transmitting and receiveing the commands between tester and driver.
 *
 * @author Onur Ulusoy
 * @date 03/02/2023
 */
#ifndef TESTERCOMMAPI_HPP
#define TESTERCOMMAPI_HPP

#include <iostream>
#include <cstring>
#include <fstream>
#include <unistd.h>
#include <chrono>
#include <time.h>
#include <sstream>
#include <iomanip>
#include <sys/stat.h>
#include "libtesterutility.hpp"
#include "CommInterface.hpp"

using namespace std;
/**
 * @namespace DriverTester
 * @brief Contains set of functions to test the driver simulates a virtual device.
 *
 */
namespace DriverTester{
    /**
     * @brief Writes a master command message to the communication register.
     * @param com An ofstream reference for the communication register file.
     * @param command The command string to be written as a master command.
     * @param file_name The name of the communication register file.
     */
    void write_master_command(std::ofstream& com, const std::string& command, const std::string& file_name);

    /**
     * @brief Writes a slave response message to the communication register.
     * @param com An ofstream reference for the communication register file.
     * @param message The message string to be written as a slave response.
     * @param file_name The name of the communication register file.
     */
    void write_slave_response(std::ofstream& com, const std::string& message, const std::string& file_name);
    /**
    @brief Evaluates the driver response to a specified command and writes the output to the log file
    @param _command A string stores the command been received from driver
    @param word The return value from driver for the command
    @param log Reference to the fstream object used to open the log file
    */
    void evaluate_slave(string _command, string word, fstream& log);
    void send_command(Publisher& publisher, const std::string& command);
    string receive_response(Subscriber& subscriber);
}

#endif //TESTERCOMMAPI_HPP
