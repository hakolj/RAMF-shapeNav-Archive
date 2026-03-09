#include "pch.h"
#include "TrainManager.h"
#include "Sensor.h"
#include "Actor.h"
#include "Random.h"
#include "Task.h"
#include "Fluid.h"
#include "Memory.h"
#include "Agent.h"
#include "Config.h"
#include "Fop.h"
#include "Timer.h"
#include "Environment.h"
#include <string>
#include <cstring>

// -c: path to config file
// -n: number of process used by openmp
int ArgHandler(int argc, char *argv[], int &num_proc, std::string &configPath, int &subCaselo, int &subCasehi)
{
    num_proc = -1;   // set to -1 by default
    configPath = ""; // set to empty by default
    subCaselo = 0;   // set to 0 by default
    subCasehi = 0;
    int i = 1;
    while (i < argc)
    {
        if ((strcmp(argv[i], "-c") == 0) && (i + 1 < argc))
        {
            // 处理 -c 参数: configpath
            configPath = std::string(argv[i + 1]);
            std::cout << "Option -c: " << configPath << std::endl;
            ++i; // 跳过下一个参数
        }
        else if ((strcmp(argv[i], "-n") == 0) && (i + 1 < argc))
        {
            // 处理 -n 参数
            try
            {
                num_proc = std::stoi(argv[i + 1]);
            }
            catch (std::exception exception)
            {
                throw(std::invalid_argument("The value of -n must be a integer!"));
            }

            std::cout << "Option -n: " << num_proc << std::endl;
            ++i; // 跳过下一个参数
        }
        else if ((strcmp(argv[i], "-sc") == 0) && (i + 1 < argc))
        {
            std::string input(argv[i + 1]);
            std::istringstream inputss(input);
            char delimiter;
            // read values
            if (inputss >> subCaselo >> delimiter >> subCasehi && (delimiter == '_'))
            {
                std::cout << "Option -sc: " << input << std::endl;
                std::cout << "Subcase range: " << subCaselo << " to " << subCasehi << std::endl;
            }
            else
            {
                throw(std::invalid_argument("The value of -sc must follow the format: idLow_idHi!"));
            }
            ++i; // 跳过下一个参数
        }

        else
        {
            // 处理其他未知参数
            std::string errmsg = "Unknown option: " + std::string(argv[i]);
            throw(std::invalid_argument(errmsg));
            return 1; // 返回非零表示发生错误
        }
        i++;
    }
    return 0;
}