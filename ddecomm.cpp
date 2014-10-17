// http://msdn.microsoft.com/en-us/library/windows/desktop/ms648711(v=vs.85).aspx
// http://msdn.microsoft.com/en-us/library/windows/desktop/ms648713(v=vs.85).aspx
// http://stackoverflow.com/questions/3306216/how-to-be-notified-of-any-update-from-dynamic-data-exchange-dde

#include <hash_map>

#include <QMutexLocker>
#include <QDebug>

#include <Windows.h>
#include <Ddeml.h>
#include <Strsafe.h>

#include "ddecomm.h"

using stdext::hash_map;

class DdeGlobal
{
private:
    QMutex mSync;
    hash_map<HCONV, DdeComm* > mConvMap;

    DdeGlobal()
        :mSync(QMutex::Recursive) {
    }

public:
    DdeComm* getComm(HCONV conversation) {
        QMutexLocker lock(&mSync);
        hash_map<HCONV, DdeComm* >::iterator it = mConvMap.find(conversation);
        if (it == mConvMap.end())
            return 0;
        else
            return it->second;
    }

    void addConv(HCONV conversation, DdeComm* comm) {
        QMutexLocker lock(&mSync);
        mConvMap[conversation] = comm;
    }

    void removeConv(HCONV conversation) {
        QMutexLocker lock(&mSync);
        DdeDisconnect(conversation);
        mConvMap.erase(conversation);
    }

    void removeAllConv(DdeComm* comm) {
        QMutexLocker lock(&mSync);
        hash_map<HCONV, DdeComm* >::iterator it = mConvMap.begin();
        while(it != mConvMap.end()) {
            if(it->second == comm)
                mConvMap.erase(it);
            ++it;
        }
    }

public:
    static DdeGlobal* getInstance() {
        static DdeGlobal inst;
        return &inst;
    }

    static HDDEDATA CALLBACK ddeCallback(
            UINT uType,     // Transaction type.
            UINT uFmt,      // Clipboard data format.
            HCONV hconv,    // Handle to the conversation.
            HSZ hsz1,       // Handle to a string depends on the type of the current transaction.
            HSZ hsz2,       // Handle to a string depends on the type of the current transaction.
            HDDEDATA hdata, // Handle to a global memory object depends on the type of the current transaction.
            DWORD dwData1,  // Transaction-specific data.
            DWORD dwData2)  // Transaction-specific data.
    {
        DdeGlobal* inst = getInstance();
        QMutexLocker lock(&inst->mSync);
        DdeComm* comm = inst->getComm(hconv);

        if (comm)
        {
            comm->log(QString("uType=%1h, uFmt=%2h, hconv=%3h, hsz1=%4h, hsz2=%5h, hdata=%6h, dwData1=%7h, dwData2=%8h")
                .arg(QString::number(uType, 16))
                .arg(QString::number(uFmt, 16))
                .arg(QString::number((int)hconv, 16))
                .arg(QString::number((int)hsz1, 16))
                .arg(QString::number((int)hsz2, 16))
                .arg(QString::number((int)hdata, 16))
                .arg(QString::number((int)dwData1, 16))
                .arg(QString::number((int)dwData2), 16));

//                    switch(uType)
//                    {
//                    case XTYP_REQUEST:
//                        printf("XTYP_REQUEST\n");
//                        break;
//                    }
        }
        else
        {
            qDebug() << "Unknown comm:" << QString("uType=%1h, uFmt=%2h, hconv=%3h, hsz1=%4h, hsz2=%5h, hdata=%6h, dwData1=%7h, dwData2=%8h")
                .arg(QString::number(uType, 16))
                .arg(QString::number(uFmt, 16))
                .arg(QString::number((int)hconv, 16))
                .arg(QString::number((int)hsz1, 16))
                .arg(QString::number((int)hsz2, 16))
                .arg(QString::number((int)hdata, 16))
                .arg(QString::number((int)dwData1, 16))
                .arg(QString::number((int)dwData2), 16);
        }

        return 0;
    }
};

// Windows specific data structure
class DdeContext
{
public:
    DdeContext(QString appName, QString topicName)
        : appName(appName.toStdWString().c_str()),
          topicName(topicName.toStdWString().c_str())    {
        instance = 0;
    }
    const std::wstring appName;
    const std::wstring topicName;

    DWORD instance;
    HSZ application;
    HSZ topic;
};

DdeComm::DdeComm(QString appName, QString topicName, QObject *parent) :
    mSync(QMutex::Recursive),
    mCtx(new DdeContext(appName, topicName)),
    QObject(parent)
{
}

DdeComm::~DdeComm()
{
    terminate();
    delete mCtx;
}

void DdeComm::initiate()
{
    QMutexLocker lock(&mSync);
    log(__FUNCTION__);

    if (mCtx->instance) {
        log(tr("Already initiated: instance=%1h").arg(QString::number(mCtx->instance, 16)));
        terminate();
    }

    DWORD inst = 0;
//    UINT result = DdeInitialize(&inst, (PFNCALLBACK) &DdeGlobal::ddeCallback, APPCLASS_STANDARD | APPCMD_CLIENTONLY ,0);
    UINT result = DdeInitialize(
            (LPDWORD) &inst,  // instance identifier
            &DdeGlobal::ddeCallback,        // pointer to callback function
            APPCLASS_MONITOR |  // this is a monitoring application
            MF_CALLBACKS     |  // monitor callback functions
            MF_CONV          |  // monitor conversation data
            MF_ERRORS        |  // monitor DDEML errors
            MF_HSZ_INFO      |  // monitor data handle activity
            MF_LINKS         |  // monitor advise loops
            MF_POSTMSGS      |  // monitor posted DDE messages
            MF_SENDMSGS,        // monitor sent DDE messages
            0);                 // reserved

    if (result == DMLERR_NO_ERROR) {
        mCtx->application = DdeCreateStringHandle(inst, mCtx->appName.c_str(), 0);
        mCtx->topic = DdeCreateStringHandle(inst, mCtx->topicName.c_str(), 0);

        if (mCtx->application && mCtx->topic) {
            mCtx->instance = inst;
            log(tr("DDE initialization succeeded: instance=%1h").arg(QString::number(inst, 16)));
        }
        else {
            log(tr("DDE not properly initiated: appName=%1, topicName=%2")
                .arg(QString::fromStdWString(mCtx->appName)).arg(QString::fromStdWString((mCtx->topicName))));
            if (mCtx->application)
                DdeFreeStringHandle(mCtx->instance, mCtx->application);
            if (mCtx->topic)
                DdeFreeStringHandle(mCtx->instance, mCtx->topic);
        }

        //        mCtx->conversation = DdeConnect(mCtx->instance, mCtx->application, mCtx->topic, NULL);
    }
    else {
        log(tr("DDE initialization failed: result=%1h").arg(QString::number(result, 16)));
    }

}

void DdeComm::terminate()
{
    QMutexLocker lock(&mSync);
    log(__FUNCTION__);

    DdeGlobal::getInstance()->removeAllConv(this);

    if (mCtx->instance)     {
        DdeFreeStringHandle(mCtx->instance, mCtx->application);
        DdeFreeStringHandle(mCtx->instance, mCtx->topic);
        DdeUninitialize(mCtx->instance);
        mCtx->instance = 0;
    }
}


























//-----------
#include <Windows.h>
#include <Ddeml.h>
#include <Strsafe.h>
#include <QThread>
#include <QEventLoop>

#include "task.h"
#include "ddecomm.h"

//    static Task* _task = 0;
//    static inline void Log(QString str)
//    {
//        if(_task)
//            _task->log(str);
//    }

//    // http://stackoverflow.com/questions/3306216/how-to-be-notified-of-any-update-from-dynamic-data-exchange-dde
//    HDDEDATA CALLBACK DdeCallback(UINT uType, UINT uFmt, HCONV hconv,
//                                  HSZ hsz1, HSZ hsz2, HDDEDATA hdata,
//                                  ULONG_PTR dwData1, ULONG_PTR dwData2)
//    {
//        printf("uType: %d", uType);
//        switch(uType)
//        {
//        case XTYP_REQUEST:
//            printf("XTYP_REQUEST\n");
//            break;

//        }
//        return 0;
//    }

//    void DDERequest(DWORD idInst, HCONV hConv, char* szItem, char* sDesc)
//    {
//        HSZ hszItem = DdeCreateStringHandleA(idInst, szItem, 0);
//        HDDEDATA hData = DdeClientTransaction(NULL,0,hConv,hszItem,CF_TEXT,
//                                              XTYP_REQUEST,5000 , NULL);
//        if (hData==NULL)
//        {
//            printf("Request failed: %s\n", szItem);
//        }
//        else
//        {
//            char szResult[255];
//            DdeGetData(hData, (unsigned char *)szResult, 255, 0);
//            printf("%s%s\n", sDesc, szResult);
//        }
//    }

void Task::run()
{
    DdeComm* mon = new DdeComm("KBSTARDS", "eds");
    connect(this, SIGNAL(destroyed()), mon, SLOT(deleteLater()));
    mon->initiate();
}

//    void Task::run()
//    {
//        ::_task = this;
//        log("Task starting");


//        char szApp[] = "MyService";
//        char szTopic[] = "Test";
//        char szCmd1[] = "[APP.MINIMIZE()]";
//        char szItem1[] = "Count";  char szDesc1[] = "A1 Contains: ";


//        UINT uiResult;
//        DWORD m_dwDDEInstance = 0;
//        uiResult = DdeInitialize(&m_dwDDEInstance, (PFNCALLBACK) &DdeCallback, APPCLASS_STANDARD|APPCMD_CLIENTONLY, 0);
//        if(uiResult != DMLERR_NO_ERROR)
//        {
//            printf("DDE Initialization Failed: 0x%04x\n", uiResult);
//        }
//        else
//        {
//            printf("m_dwDDEInstance: %u\n", m_dwDDEInstance);
//            //PAUSE;
//            HSZ hszApp, hszTopic;
//            HCONV hConv;
//            hszApp = DdeCreateStringHandleA(m_dwDDEInstance, szApp, 0);
//            hszTopic = DdeCreateStringHandleA(m_dwDDEInstance, szTopic, 0);
//            hConv = DdeConnect(m_dwDDEInstance, hszApp, hszTopic, NULL);
//            DdeFreeStringHandle(m_dwDDEInstance, hszApp);
//            DdeFreeStringHandle(m_dwDDEInstance, hszTopic);
//            if(hConv == NULL)
//            {
//                printf("DDE Connection Failed.\n");
//            }

//            DDERequest(m_dwDDEInstance, hConv, szItem1, szDesc1);
//            DdeDisconnect(hConv);
//            DdeUninitialize(m_dwDDEInstance);
//        }

//        log("Task ended");
//        finished();
//    }




//    //#include "stdafx.h"
//    #include "windows.h"
//    #include "ddeml.h"
//    #include "stdio.h"

//    char szApp[] = "EXCEL";
//    char szTopic[] = "C:\\Test.xls";
//    char szCmd1[] = "[APP.MINIMIZE()]";
//    char szItem1[] = "R1C1";  char szDesc1[] = "A1 Contains: ";
//    char szItem2[] = "R2C1";  char szDesc2[] = "A2 Contains: ";
//    char szItem3[] = "R3C1";  char szData3[] = "Data from DDE Client";

//    HSZ hszApp, hszTopic;
//    DWORD idInst=0;

//    HDDEDATA CALLBACK DdeCallback(
//        UINT uType,     // Transaction type.
//        UINT uFmt,      // Clipboard data format.
//        HCONV hconv,    // Handle to the conversation.
//        HSZ hsz1,       // Handle to a string.
//        HSZ hsz2,       // Handle to a string.
//        HDDEDATA hdata, // Handle to a global memory object.
//        DWORD dwData1,  // Transaction-specific data.
//        DWORD dwData2)  // Transaction-specific data.
//    {
//        if(uType==XTYP_ADVDATA && uFmt==CF_TEXT)
//        {
//            HSZ hszItem1 = DdeCreateStringHandle(idInst, szItem1, 0);
//            HSZ hszItem2 = DdeCreateStringHandle(idInst, szItem2, 0);
//            char szResult[255];
//            if((!DdeCmpStringHandles(hsz1, hszTopic)) && (!DdeCmpStringHandles(hsz2, hszItem1)))
//            {
//                DdeGetData(hdata, (unsigned char *)szResult, 255, 0);
//                printf("%s - %s\n", szItem1,szResult);
//            }
//            else if((!DdeCmpStringHandles(hsz1, hszTopic)) && (!DdeCmpStringHandles(hsz2, hszItem2)))
//            {
//                DdeGetData(hdata, (unsigned char *)szResult, 255, 0);
//                printf("%s - %s\n", szItem2,szResult);
//            }
//        }
//        return 0;
//    }

//    void DDEExecute(DWORD idInst, HCONV hConv, char* szCommand)
//    {
//        HDDEDATA hData = DdeCreateDataHandle(idInst, (LPBYTE)szCommand,
//                                   lstrlen(szCommand)+1, 0, NULL, CF_TEXT, 0);
//        if (hData==NULL)   {
//            printf("Command failed: %s\n", szCommand);
//        }
//        else    {
//            DdeClientTransaction((LPBYTE)hData, 0xFFFFFFFF, hConv, 0L, 0,
//                                 XTYP_EXECUTE, TIMEOUT_ASYNC, NULL);
//        }
//    }

//    void DDERequest(DWORD idInst, HCONV hConv, char* szItem, char* sDesc)
//    {
//        HSZ hszItem = DdeCreateStringHandle(idInst, szItem, 0);
//        HDDEDATA hData = DdeClientTransaction(NULL,0,hConv,hszItem,CF_TEXT,
//                                     XTYP_REQUEST,5000 , NULL);
//        if (hData==NULL)
//        {
//            printf("Request failed: %s\n", szItem);
//        }
//        else
//        {
//            char szResult[255];
//            DdeGetData(hData, (unsigned char *)szResult, 255, 0);
//            printf("%s%s\n", sDesc, szResult);
//        }
//    }

//    void DDEPoke(DWORD idInst, HCONV hConv, char* szItem, char* szData)
//    {
//        HSZ hszItem = DdeCreateStringHandle(idInst, szItem, 0);
//        DdeClientTransaction((LPBYTE)szData, (DWORD)(lstrlen(szData)+1),
//                              hConv, hszItem, CF_TEXT,
//                              XTYP_POKE, 3000, NULL);
//        DdeFreeStringHandle(idInst, hszItem);
//    }



//    int main(int argc, char* argv[])
//    {

//        char szCmd2[] = "[SELECT(\"R3C1\")][FONT.PROPERTIES(,\"Bold\")]";//[SAVE()][QUIT()]";

//        //DDE Initialization
//        UINT iReturn;
//        iReturn = DdeInitialize(&idInst, (PFNCALLBACK)DdeCallback,
//                                APPCLASS_STANDARD | APPCMD_CLIENTONLY, 0 );
//        if (iReturn!=DMLERR_NO_ERROR)
//        {
//            printf("DDE Initialization Failed: 0x%04x\n", iReturn);
//            Sleep(1500);
//            return 0;
//        }

//        //Start DDE Server and wait for it to become idle.
//        HINSTANCE hRet = ShellExecute(0, "open", szTopic, 0, 0, SW_SHOWNORMAL);
//        if ((int)hRet < 33)
//        {
//            printf("Unable to Start DDE Server: 0x%04x\n", (UINT)hRet);
//            Sleep(1500); DdeUninitialize(idInst);
//            return 0;
//        }
//        Sleep(1000);

//        //DDE Connect to Server using given AppName and topic.
//        HCONV hConv;
//        hszApp = DdeCreateStringHandle(idInst, szApp, 0);
//        hszTopic = DdeCreateStringHandle(idInst, szTopic, 0);
//        hConv = DdeConnect(idInst, hszApp, hszTopic, NULL);
//        DdeFreeStringHandle(idInst, hszApp);
//        DdeFreeStringHandle(idInst, hszTopic);
//        if (hConv == NULL)
//        {
//            printf("DDE Connection Failed.\n");
//            Sleep(1500); DdeUninitialize(idInst);
//            return 0;
//        }

//        //Execute commands/requests specific to the DDE Server.
//        DDEExecute(idInst, hConv, szCmd1);
//        DDERequest(idInst, hConv, szItem1, szDesc1);
//        DDERequest(idInst, hConv, szItem2, szDesc2);
//        DDEPoke(idInst, hConv, szItem3, szData3);
//        DDEExecute(idInst, hConv, szCmd2);

//        HSZ hszItem = DdeCreateStringHandle(idInst, szItem1, 0);
//        DdeClientTransaction(NULL,0,hConv,hszItem,CF_TEXT,
//                                     XTYP_ADVSTART,5000 , NULL);


//    BOOL bRet;
//    MSG msg;

//    while( (bRet = GetMessage( &msg, NULL, 0, 0 )) != 0)
//    {
//        if (bRet == -1)
//        {
//            // handle the error and possibly exit
//        }
//        else
//        {
//            TranslateMessage(&msg);
//            DispatchMessage(&msg);
//        }
//    }

//        //DDE Disconnect and Uninitialize.
//        DdeDisconnect(hConv);
//        DdeUninitialize(idInst);

//        Sleep(3000);
//        return 1;
//    }
