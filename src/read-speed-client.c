//
// Created by profanter on 04/10/2019.
// Copyright (c) 2019 fortiss GmbH. All rights reserved.
//

#include <open62541/plugin/log_stdout.h>
#include <open62541/client.h>
#include <open62541/client_config.h>
#include <open62541/client_highlevel.h>
#include <open62541/client_config_default.h>
#include <time.h>


static volatile UA_Boolean running = true;
static void stopHandler(int sig) {
    UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "received ctrl-c");
    running = false;
}

UA_Variant readNanosecondsVariant;

UA_UInt64 readNanosecondsValueLoop(UA_Client* client) {
    UA_StatusCode retval = UA_Client_readValueAttribute(client, UA_NODEID_NUMERIC(1, 42), &readNanosecondsVariant);
    if(retval == UA_STATUSCODE_GOOD && UA_Variant_isScalar(&readNanosecondsVariant) &&
            readNanosecondsVariant.type == &UA_TYPES[UA_TYPES_UINT64]) {
        return *(UA_UInt64*)readNanosecondsVariant.data;
    } else {
        printf("ERROR: Could not read variable: %s", UA_StatusCode_name(retval));
    }
    return 0;
}

int main(int argc, char *argv[]) {
    signal(SIGINT, stopHandler);
    signal(SIGTERM, stopHandler);


    if (argc != 2) {
        printf("usage: client [ENDPOINT_URL]");
        return EXIT_FAILURE;
    }

    UA_Client *client = UA_Client_new();
    UA_ClientConfig_setDefault(UA_Client_getConfig(client));

    printf("Connecting to server: %s", argv[1]);
    /* Connect to a server */
    /* anonymous connect would be: retval = UA_Client_connect(client, "opc.tcp://localhost:4840"); */
    UA_StatusCode retval = UA_Client_connect(client, argv[1]);
    if(retval != UA_STATUSCODE_GOOD) {
        printf("ERROR: Could not connect to server.");
        UA_Client_delete(client);
        return EXIT_FAILURE;
    }

    double lastPrintTime;
    struct timespec tNow;
    clock_gettime(CLOCK_MONOTONIC, &tNow);
    lastPrintTime = (double)tNow.tv_sec + (double)tNow.tv_nsec / 1000000000.0;

    int counter = 0;

    UA_UInt64 nsMin = UA_UINT32_MAX;
    UA_UInt64 nsMax = 0;
    UA_UInt64 nsSum = 0;

    printf("------ Starting reading loop. ------\n");
    printf("readPerSec\t readTimeAvg\treadTimeMin\t  readTimeMax\n");

    UA_UInt64 nsPrevious = readNanosecondsValueLoop(client);
    while(running) {
        UA_UInt64 nsNow = readNanosecondsValueLoop(client);
        UA_UInt64 ns = nsNow - nsPrevious;
        nsPrevious = nsNow;
        if (ns < nsMin)
            nsMin = ns;
        if (ns > nsMax)
            nsMax = ns;
        nsSum += ns;
        counter++;

        // to save calculation time, just check every x iterations
        if (counter == 100000) {
            double nowTime;
            clock_gettime(CLOCK_MONOTONIC, &tNow);
            nowTime = (double)tNow.tv_sec + (double)tNow.tv_nsec / 1000000000.0;

            double secondsDiff = nowTime - lastPrintTime;
            lastPrintTime = nowTime;
            double nsAvg = nsSum / (double) counter;
            double readPerSec = counter / secondsDiff;

            printf("%2.2f\t%6.0fns\t%9ldns\t\t%9ldns\n", readPerSec, nsAvg, nsMin, nsMax);
            counter = 0;
            nsSum = 0;
            nsMin = UA_UINT32_MAX;
            nsMax = 0;
            // printing the log takes some time. Reset values to ensure correct measuring
            clock_gettime(CLOCK_MONOTONIC, &tNow);
            lastPrintTime = (double)tNow.tv_sec + (double)tNow.tv_nsec / 1000000000.0;
            nsPrevious = readNanosecondsValueLoop(client);
        }

    }

    UA_Client_disconnect(client);
    UA_Client_delete(client);
    return EXIT_SUCCESS;
}