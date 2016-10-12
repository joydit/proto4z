﻿


#include "any.h"
#include "common.h"
#include "genBase.h"
#include "parseCache.h"
#include "parseProto.h"
std::map<std::string, unsigned short> _cacheKeys;
std::map<std::string, std::string> _errCodes;
//#define __WITH_TAG
int main(int argc, char *argv[])
{
    zsummer::log4z::ILog4zManager::getRef().setLoggerFileLine(LOG4Z_MAIN_LOGGER_ID, false);
    zsummer::log4z::ILog4zManager::getRef().setLoggerOutFile(LOG4Z_MAIN_LOGGER_ID, false);
    zsummer::log4z::ILog4zManager::getRef().start();
    std::vector<SearchFileInfo> files;
    if (!searchFiles("./*.xml", files, false))
    {
        LOGE("searchFiles error.");
        return 0;
    }
    try
    {
        bool updateReflection = false;
        for (auto & file : files)
        {
            std::string filename = subStringWithoutBack(file.filename, ".");
            ParseCache cache;
            cache.parse(filename);
            if (cache.isNeedUpdate())
            {
                updateReflection = true;
                auto stores = parseProto(filename, cache);
                for (int i = SL_NORMAL + 1; i < SL_END; i++)
                {
                    auto gen = createGenerate((SupportLanguageType)i);
                    if (!gen)
                    {
                        continue;
                    }
                    gen->init(filename, (SupportLanguageType)i);
                    auto content = gen->genRealContent(stores);
                    gen->write(content);
                    destroyGenerate(gen);
                }
                cache.write();
            }
            if (true)
            {
                auto stores = parseProto(filename, cache);
                for (auto & store : stores)
                {
                    if (store._type == GT_DataEnum && trim(store._enum._name) == "ERROR_CODE")
                    {
                        for (auto & kv : store._enum._members)
                        {
                            _errCodes[kv._value] = kv._desc;
                        }
                    }
                }
            }
            _cacheKeys.insert(cache._cacheNumber.begin(), cache._cacheNumber.end());
        }
        if (updateReflection)
        {
            writeCSharpReflection(_cacheKeys, _errCodes);
            writeCPPReflection(_cacheKeys, _errCodes);
        }

    }
    catch (const std::exception & e)
    {
        LOGE("catch error: " << e.what());
        return -1;
    }

    
    LOGA("All Success..");

    return 0;
}






