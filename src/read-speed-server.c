/* This is free and unencumbered software released into the public domain.
 * See LICENSE file for more information */


#include <open62541/plugin/log_stdout.h>
#include <open62541/server.h>
#include <open62541/server_config_default.h>

#include <signal.h>
#include <stdlib.h>
#include <time.h>

static volatile UA_Boolean running = true;
static void stopHandler(int sig) {
    UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "received ctrl-c");
    running = false;
}

UA_StatusCode iterateNoWaitInternal(UA_Server *server) {
    UA_StatusCode retval = UA_Server_run_startup(server);
    if(retval != UA_STATUSCODE_GOOD)
        return retval;
    while(running) {
        UA_Server_run_iterate(server, false);
    }
    retval = UA_Server_run(server, &running);
    if(retval != UA_STATUSCODE_GOOD)
        return retval;

    return UA_STATUSCODE_GOOD;
}

static UA_StatusCode
readCurrentNanoseconds(UA_Server *server,
                const UA_NodeId *sessionId, void *sessionContext,
                const UA_NodeId *nodeId, void *nodeContext,
                UA_Boolean sourceTimeStamp, const UA_NumericRange *range,
                UA_DataValue *dataValue) {

    struct timespec tnow={0,0};
    clock_gettime(CLOCK_MONOTONIC, &tnow);

    struct tm *my_tm = localtime(&tnow.tv_sec);

    UA_UInt64 seconds = ((my_tm->tm_hour * 60) + my_tm->tm_min) * 60 + my_tm->tm_sec;
    UA_UInt64 nanosecsSinceMidnight = seconds * 1000 * 1000 * 1000 + tnow.tv_nsec;

    UA_Variant_setScalarCopy(&dataValue->value, &nanosecsSinceMidnight,
                             &UA_TYPES[UA_TYPES_UINT64]);
    dataValue->hasValue = true;
    return UA_STATUSCODE_GOOD;
}

static UA_StatusCode
addCurrentNanosecondsVariable(UA_Server *server) {
    UA_VariableAttributes attr = UA_VariableAttributes_default;
    attr.displayName = UA_LOCALIZEDTEXT("en-US", "Current Nanoseconds of the day");
    attr.accessLevel = UA_ACCESSLEVELMASK_READ;

    UA_NodeId currentNodeId = UA_NODEID_NUMERIC(1, 42);
    UA_QualifiedName currentName = UA_QUALIFIEDNAME(1, "current-time-nanoseconds");
    UA_NodeId parentNodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER);
    UA_NodeId parentReferenceNodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES);
    UA_NodeId variableTypeNodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE);

    UA_DataSource timeDataSource;
    timeDataSource.read = readCurrentNanoseconds;
    return UA_Server_addDataSourceVariableNode(server, currentNodeId, parentNodeId,
                                        parentReferenceNodeId, currentName,
                                        variableTypeNodeId, attr,
                                        timeDataSource, NULL, NULL);
}

int main(void) {
    signal(SIGINT, stopHandler);
    signal(SIGTERM, stopHandler);

    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);

    UA_Server *server = UA_Server_new();
    UA_ServerConfig_setDefault(UA_Server_getConfig(server));

    UA_StatusCode retval = addCurrentNanosecondsVariable(server);
    if(retval != UA_STATUSCODE_GOOD) {
        printf("ERROR: Could not add variable! %s", UA_StatusCode_name(retval));
    }


    retval = iterateNoWaitInternal(server);
    if(retval != UA_STATUSCODE_GOOD) {
        printf("ERROR: Could not start server! %s", UA_StatusCode_name(retval));
    }

    UA_Server_delete(server);
    return retval == UA_STATUSCODE_GOOD ? EXIT_SUCCESS : EXIT_FAILURE;
}