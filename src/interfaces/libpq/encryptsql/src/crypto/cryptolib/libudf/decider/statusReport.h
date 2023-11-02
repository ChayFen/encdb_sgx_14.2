#pragma once

#include <stdint.h>
#include <string.h>

typedef enum UdfKind
{
    SAHE_ADD,   // cipher + cipher
    SAHE_MUL,   // cipher * plain
    SAHE_SUB,   // cipher - cipher
    SAHE_NEG,   // - cipher

    SMHE_MUL,   // cipher * cipher
    SMHE_MULP,  // cipher * plain
    SMHE_DIV,   // cipher / cipher
    SMHE_POW,   // pow(cipher,n)
    SMHE_INV,   // 1 / cipher

    ORE,
    DET,
    SUM,
} UdfKind;

typedef enum ExecutionPath
{
    SOFTWARE,
    HARDWARE,
    COUNT,
    NOTSET
} ExecutionPath;

typedef struct StatusReport
{
    enum UdfKind _udfKind;
    double _runningTime;
    size_t _processedDataSize;
    enum ExecutionPath _path;
    double _enclaveMemoryAmout; // 运行之前的enclave内存容量
    StatusReport(UdfKind udfKind, size_t dataSize, double runningTime = -1, ExecutionPath path = NOTSET, double epcUsage = -1)
        : _udfKind(udfKind), _runningTime(runningTime), _processedDataSize(dataSize), _path(path), _enclaveMemoryAmout(epcUsage)
    {}
} StatusReport;

// StatusReport *getReport(UdfKind kind, size_t dataSize, double EPCAmount, ExecutionPath path, double runningtime)
// {
//     StatusReport *rep = (StatusReport*) malloc(sizeof(StatusReport));
//     rep->_enclaveMemoryAmout = EPCAmount;
//     rep->_udfKind = kind;
//     rep->_processedDataSize = dataSize;
//     rep->_path = path;
//     rep->_runningTime = runningtime;
//     return rep;
// }
