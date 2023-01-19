#include "gpiolib.h"

fstream& GotoLine(std::fstream& file, unsigned int num){
    file.seekg(std::ios::beg);
    for(int i=0; i < num - 1; ++i){
        file.ignore(std::numeric_limits<std::streamsize>::max(),'\n');
    }
    return file;
}

void Split(string s, string del, string* buffers)
{
    //string buffers [8];
    int i = 0;

    int start, end = -1*del.size();
    do {
        start = end + del.size();
        end = s.find(del, start);
        buffers[i] = s.substr(start, end - start);
        i++;
    } while (end != -1);
}

static inline std::string &rtrim(std::string &s) {
    s.erase(std::find_if(s.rbegin(), s.rend(),
                         std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
    return s;
}

GPIO_Device::GPIO_Device(const char *dev_name) {
    this->dev_name = dev_name;
    hist.open("history", ios::app);
    //cout << this->dev_name << endl;
}

void GPIO_Device::device_open(command request, GPIO_Device* gpioDevHandler) {

    switch (request) {

        case READONLY:
            cout << "function 'GPIO_Device::device_open' worked as READONLY" << endl;


            fd.open(dev_name, ios::in);
            //cout << fd.is_open() << endl;

            if (!fd.is_open()){
                GPIO_Device::device_open(WRITEONLY, gpioDevHandler);

                devContent.fill(DEFAULT, gpioDevHandler);

                GPIO_Device::device_close();
                GPIO_Device::device_open(READONLY, gpioDevHandler);
                break;

            }

            if (!fd.is_open())
            {
                printf("Unabled to open %s: %s\n", dev_name, strerror(errno));
                break;
            }

            else
                cout << dev_name << " is opened successfully as READONLY" <<endl;

            cout << endl;
            break;

        case WRITEONLY:
            cout << "function 'GPIO_Device::device_open' worked as WRITEONLY" << endl;

            fd.open(dev_name, ios::out | ios::trunc);

            if (!fd.is_open())
            {
                printf("Unabled to open %s: %s\n", dev_name, strerror(errno));
                break;
            }

            else
                cout << dev_name << " is opened successfully as WRITEONLY" << endl;

            cout << endl;
            break;

        default:
            cout << "function 'GPIO_Device::device_open' worked as DEFAULT" << endl << endl;
            //device_close();
            fd.open(gpioDevHandler->dev_name);

            if (!fd.is_open())
            {
                printf("Unabled to open %s: %s\n", dev_name, strerror(errno));
                break;
            }

            else
                cout << dev_name << " is opened successfully as DEFAULT" << endl;

            cout << endl;
            break;

    }
}

void GPIO_Device::device_close() {

    cout << "function 'GPIO_Device::device_close' worked" << endl;

    if (fd.is_open()){
        cout << dev_name << " is closed successfully" << endl;
        fd.close();
    }

    else{
        //fd = 0;
        cout << dev_name << " is already not open" << endl;
    }

    cout << endl;
}

void GPIO_Device::DeviceContent::fill(command request, GPIO_Device* gpioDevHandler) {

    cout << "function 'GPIO_Device::DeviceContent::fill' worked" << endl;

    gpioDevHandler->device_open(WRITEONLY, gpioDevHandler);

    if (request == DEFAULT){

        std::ifstream jsonFile("dev/default_chipInfo.json");
        nlohmann::json commands;
        jsonFile >> commands;

        nlohmann::json data = commands["gpioDevices"];
        unsigned long dataSize = data.size();

        cout << "Number of items in gpioDevices: " << data.size() << endl << endl;

        for (int i = 0; i<dataSize; i++) {

            gpioDevHandler->fd << data.at(i).value("offset", 0) << " ";
            gpioDevHandler->fd << data.at(i).value("name", "-") << " ";
            gpioDevHandler->fd << data.at(i).value("consumer", "-") << " ";
            gpioDevHandler->fd << data.at(i).value("FLAG_IS_OUT", "-") << " ";
            gpioDevHandler->fd << data.at(i).value("FLAG_ACTIVE_LOW", "-") << " ";
            gpioDevHandler->fd << data.at(i).value("FLAG_OPEN_DRAIN", "-") << " ";
            gpioDevHandler->fd << data.at(i).value("FLAG_OPEN_SOURCE", "-") << " ";
            gpioDevHandler->fd << data.at(i).value("FLAG_KERNEL", "-");

            gpioDevHandler->fd << endl;

             /*
            chipxInfo[i] = {
                    .offset = data.at(i).value("offset", 0),
                    .name = data.at(i).value("name", "-"),
                    .consumer = data.at(i).value("consumer", "-"),
                    .FLAG_IS_OUT = data.at(i).value("FLAG_IS_OUT", "-"),
                    .FLAG_ACTIVE_LOW = data.at(i).value("FLAG_ACTIVE_LOW", "-"),
                    .FLAG_OPEN_DRAIN = data.at(i).value("FLAG_OPEN_DRAIN", "-"),
                    .FLAG_OPEN_SOURCE = data.at(i).value("FLAG_OPEN_SOURCE", "-"),
                    .FLAG_KERNEL = data.at(i).value("FLAG_KERNEL", "-")
            };
            */


        }

        cout << "Default chip info is written to the file successfully" << endl;
        gpioDevHandler->hist << "Default chip info is written to the file '" << gpioDevHandler->dev_name << "' successfully" << endl;


    }

    gpioDevHandler->device_close();
}

void GPIO_Device::DeviceContent::show(GPIO_Device* gpioDevHandler) {

    cout << "function 'GPIO_Device::DeviceContent::show' worked" << endl << endl;
    //gpioDevHandler->fd.open(gpioDevHandler->dev_name, ios::in);

    gpioDevHandler->device_open(READONLY, gpioDevHandler);

    cout << "Chip info is being shown ..." << endl << endl;

    // Get the maximum length of each column
    int max_offset_length = 0;
    int max_name_length = 0;
    int max_consumer_length = 0;
    int max_flags_length = 0;

    while (true) {
        string line, word;
        for (int i=0; i<8;i++){
            gpioDevHandler->fd >> word;
            line += word + "/";
        }

        if( gpioDevHandler->fd.eof() ) break;

        auto* buffers = new string[9];
        Split(line, "/", buffers);

        int offset_length = buffers[0].length();
        if (offset_length > max_offset_length) max_offset_length = offset_length;

        int name_length = buffers[1].length();
        if (name_length > max_name_length) max_name_length = name_length;

        int consumer_length = buffers[2].length();
        if (consumer_length > max_consumer_length) max_consumer_length = consumer_length;

        int flags_length = 0;
        for (int i=3; i<8; i++){
            flags_length += buffers[i].length();
        }
        if (flags_length > max_flags_length) max_flags_length = flags_length;

        delete[] buffers;
    }

    gpioDevHandler->fd.clear();
    gpioDevHandler->fd.seekg(0, ios::beg);

    gpioDevHandler->hist << "Data was shown for '" << gpioDevHandler->dev_name << "':" << endl;
    while (true) {
        string line, word;
        for (int i=0; i<8;i++){
            gpioDevHandler->fd >> word;
            line += word + "/";
        }

        if( gpioDevHandler->fd.eof() ) break;

        auto* buffers = new string[9];
        Split(line, "/", buffers);

        cout << "Offset: ";
        cout << setw(max_offset_length) << left << buffers[0];

        gpioDevHandler->hist << "Offset: ";
        gpioDevHandler->hist << setw(max_offset_length) << left << buffers[0];

        cout << "\t Name: ";
        cout << setw(max_name_length) << left << buffers[1];

        gpioDevHandler->hist << "\t Name: ";
        gpioDevHandler->hist << setw(max_name_length) << left << buffers[1];

        cout << "\tConsumer: ";
        cout << setw(max_consumer_length) << left << buffers[2];

        gpioDevHandler->hist << "\tConsumer: ";
        gpioDevHandler->hist << setw(max_consumer_length) << left << buffers[2];

        cout << "\t Flags: ";
        gpioDevHandler->hist << "\t Flags: ";

        for (int i=3; i<8; i++){
            cout << " " << buffers[i];
            gpioDevHandler->hist << " " << buffers[i];
        }

        cout << endl;
        gpioDevHandler->hist << endl;
        delete[] buffers;
    }

    cout << endl;
    cout << "Chip info is shown successfully" << endl;
}

string GPIO_Device::DeviceContent::read(int offset, enum feature request, GPIO_Device* gpioDevHandler){

    cout << "function 'GPIO_Device::DeviceContent::read' worked" << endl << endl;

    gpioDevHandler->device_open(DEFAULT, gpioDevHandler);

    GotoLine(gpioDevHandler->fd, offset+1);

    string word, empty;

    for (int i=0; i<request+1;i++){
        gpioDevHandler->fd >> empty;
    }
    gpioDevHandler->fd >> word;

    cout << endl;
    cout << "Chip info is read successfully" << endl << endl;

    cout << "Data has been stored: " << word << endl;

    gpioDevHandler->hist << "Data has been stored: " << word << endl;

    return word;
}

void GPIO_Device::DeviceContent::write (int offset, enum feature request, string new_value, GPIO_Device* gpioDevHandler){

    //cout << gpioDevHandler->dev_name << endl;
    cout << "function 'GPIO_Device::DeviceContent::write' worked" << endl << endl;

    //cout << gpioDevHandler->dev_name << endl;

    gpioDevHandler->device_open(DEFAULT, gpioDevHandler);

    GotoLine(gpioDevHandler->fd, offset+1);

    string line;
    getline(gpioDevHandler->fd, line);
    //cout << line << endl;

    auto* buffers = new string[9];
    Split(line, " ", buffers);

    string new_line;

    for (int i=0; i<8; i++){

        if (i == request+1){
            new_line += new_value + " ";
        }

        else
        new_line += buffers[i] + " ";
    }
    rtrim(new_line);
    //cout << new_line << endl << endl;


    GotoLine(gpioDevHandler->fd, 1);

    fstream newfile;
    ofstream outfile (NEW_FILE);
    outfile.close();

    newfile.open(NEW_FILE);
    int i = 0;

    while (true) {

        if (i != offset){
            getline(gpioDevHandler->fd, line);
            newfile << line << endl;
        }

        else {
            newfile << new_line << endl;
            GotoLine(gpioDevHandler->fd, i+2);
        }

        i++;
        if( gpioDevHandler->fd.eof() ) break;
    }

    newfile.close();

    gpioDevHandler->device_close();

    remove(gpioDevHandler->dev_name);
    rename(NEW_FILE, gpioDevHandler->dev_name);

    cout << endl;
    cout << "Chip info is changed successfully" << endl;
    gpioDevHandler->hist << "Chip info is changed successfully" << endl;
}

/*
int GPIO_Device::device_write(int offset, uint8_t value)
{
    printf("Write value %d to GPIO at offset %d (OUTPUT mode) on chip %s\n", value, offset, dev_name);

    if (fd <= 0)
        device_open();

    rq.lineoffsets[0] = offset;

    rq.flags = GPIOHANDLE_REQUEST_OUTPUT;

    rq.lines = 1;

    ret = ioctl(fd, GPIO_GET_LINEHANDLE_IOCTL, &(rq));

    device_close();

    if (ret == -1)
    {
        printf("Unable to line handle from ioctl : %s\n", strerror(errno));
        return 0;
    }

    data.values[0] = value;

    ret = ioctl(rq.fd, GPIOHANDLE_SET_LINE_VALUES_IOCTL, &data);

    if (ret == -1)
    {
        printf("Unable to set line value using ioctl : %s\n", strerror(errno));
    }

    else

    {

        usleep(2000000);

    }

    close(rq.fd);
    return 0;
}

/*
int GPIO_Device::gpio_list()
{
    if (fd <= 0)
        device_open();

    ret = ioctl(fd, GPIO_GET_CHIPINFO_IOCTL, &info);

    if (ret == -1)
    {

        printf("Unable to get chip info from ioctl: %s\n", strerror(errno));

        device_close();

        return 0;
    }

    printf("Chip name: %s\n", info.name);

    printf("Chip label: %s\n", info.label);

    printf("Number of lines: %d\n", info.lines);


    for (int i = 0; i < info.lines; i++)
    {

        line_info.line_offset = i;

        ret = ioctl(fd, GPIO_GET_LINEINFO_IOCTL, &line_info);

        if (ret == -1)
        {
            printf("Unable to get line info from offset %d: %s\n", i, strerror(errno));
        }

        else
        {

            printf("offset: %d, name: %s, consumer: %s. Flags:\t[%s]\t[%s]\t[%s]\t[%s]\t[%s]\n",

                   i,

                   line_info.name,

                   line_info.consumer,

                   (line_info.flags & GPIOLINE_FLAG_IS_OUT) ? "OUTPUT" : "INPUT",

                   (line_info.flags & GPIOLINE_FLAG_ACTIVE_LOW) ? "ACTIVE_LOW" : "ACTIVE_HIGHT",

                   (line_info.flags & GPIOLINE_FLAG_OPEN_DRAIN) ? "OPEN_DRAIN" : "...",

                   (line_info.flags & GPIOLINE_FLAG_OPEN_SOURCE) ? "OPENSOURCE" : "...",

                   (line_info.flags & GPIOLINE_FLAG_KERNEL) ? "KERNEL" : "");

        }

    }

    device_close();

    return 0;
}





int GPIO_Device::gpio_read(int offset) {

    if (fd <= 0)
        device_open();

    rq.lineoffsets[0] = offset;

    rq.flags = GPIOHANDLE_REQUEST_INPUT;

    rq.lines = 1;

    ret = ioctl(fd, GPIO_GET_LINEHANDLE_IOCTL, &rq);

    device_close();

    if (ret == -1) {
        printf("Unable to get line handle from ioctl : %s", strerror(errno));

        return 0;
    }

    ret = ioctl(rq.fd, GPIOHANDLE_GET_LINE_VALUES_IOCTL, &data);

    if (ret == -1) {
        printf("Unable to get line value using ioctl : %s", strerror(errno));
    } else {
        printf("Value of GPIO at offset %d (INPUT mode) on chip %s: %d\n", offset, dev_name, data.values[0]);
    }

    close(rq.fd);
    return 0;
}
 */