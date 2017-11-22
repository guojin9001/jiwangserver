#include "mainwindow.h"
#include "ui_mainwindow.h"
int MainWindow::ConNum=0;
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    connect(this,SIGNAL(valuechanged(QString)),this,SLOT(uichange(QString)));
}
void MainWindow::uichange(QString str)
{
    ui->textBrowser->append(str);
}
void MainWindow::initsock()
{
    if(WSAStartup(0x0101,&wsaData))
    {
    ui->textBrowser->append("Server initialize winsock error!\n");
    return;
    }
    if(wsaData.wVersion != 0x0101)
    {
    ui->textBrowser->append("Server's winsock version error!\n");
    WSACleanup();
    return;
    }
    ui->textBrowser->append("Server's winsock initialized !\n");

    //创建 TCP socket
    SerSocket = socket(AF_INET,SOCK_STREAM,0);
    if(SerSocket == INVALID_SOCKET)
    {
    ui->textBrowser->append("Server create socket error!\n");
    WSACleanup();
    return;
    }
    ui->textBrowser->append("Server TCP socket create OK!\n");

    //绑定 socket to Server's IP and port 5050
    srvAddr.sin_family = AF_INET;
    srvAddr.sin_port = htons(5050);
    srvAddr.sin_addr.S_un.S_addr = INADDR_ANY;
    if(bind(SerSocket,(LPSOCKADDR)&srvAddr,sizeof(srvAddr))== SOCKET_ERROR)
    {
    ui->textBrowser->append("Server socket bind error!\n");
    closesocket(SerSocket);
    WSACleanup();
    return;
    }
    ui->textBrowser->append("Server socket bind OK!\n");

    if(listen(SerSocket,MAXCONN) == SOCKET_ERROR)
    {
    printf("Server socket listen error!\n");
    closesocket(SerSocket);
    WSACleanup();
    return;
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_pushButton_clicked()
{
    initsock();
    par *pa=new par;
    pa->m=this;
    pa->s=SerSocket;
   CreateThread(0, 0, (LPTHREAD_START_ROUTINE)AcceptProc,(LPVOID*)pa, 0, 0);
}
DWORD WINAPI clientProc(LPARAM lparam)
{
    par2 *q=new par2;
    q->m=((par2 *)lparam)->m;
    q->s=((par2 *)lparam)->s;
    q->clientAddr=((par2 *)lparam)->clientAddr;
    char *ch=inet_ntoa(q->clientAddr.sin_addr);
    QString ipadd= QString(QLatin1String(ch));
    QString str,str1,str2,str3;
    QString retUsrName;


    emit q->m->valuechanged(ipadd);
    SOCKET sockClient=q->s;
    char buf[1024];
    // 以上是线程参数的传递以及初始化


    //以下是创建数据库连接
    QSqlDatabase database;
    if (QSqlDatabase::contains("myconnetcion"+QString::number(MainWindow::ConNum)))
    {
        database = QSqlDatabase::database("myconnetcion"+QString::number(MainWindow::ConNum));
        emit q->m->valuechanged("database has been created");
    }
    else
    {
        database = QSqlDatabase::addDatabase("QSQLITE","myconnetcion"+QString::number(MainWindow::ConNum));
        database.setDatabaseName("C:\\Users\\jinguo\\Documents\\build-server-Desktop_Qt_5_9_2_MSVC2015_32bit-Debug\\MyDataBase.db");
        database.setUserName("jinguo");
    }
    (MainWindow::ConNum)++;
    if(!database.open())
    {
        emit q->m->valuechanged("database open error");
    }
    else
    {
        QSqlQuery query;
        query=QSqlQuery::QSqlQuery(database);//
        QString create_sql ="create table userinfo(name varchar(20), password varchar(20),ipaddr varchar(20),lixian varchar(50),status varchar(20),telephone varchar(20))";
        query.prepare(create_sql);
        if(!query.exec())
        {
            emit q->m->valuechanged( "Error: Fail to create table.");
            query.lastError();
        }
        else
        {
           emit q->m->valuechanged("Table created!");
        }
        //以下是对接受的数据进行判断

        while (1)
        {
            memset(buf, 0, sizeof(buf));
            // 接收客户端的一条数据
            int ret = recv(sockClient, buf, sizeof(buf), 0);

            if(ret==SOCKET_ERROR)
            {
                closesocket(sockClient);
                return -1;
            }//表示用户断开连接
            str= QString(QLatin1String(buf));
            emit q->m->valuechanged(str);

            switch(str.toInt())
            {
            case 0 : {
                       recv(sockClient, buf, sizeof(buf), 0);
                       str1= QString(QLatin1String(buf));
                       emit q->m->valuechanged(str1);


                       recv(sockClient, buf, sizeof(buf), 0);
                       str2= QString(QLatin1String(buf));
                       emit q->m->valuechanged(str2);

                       QString select_sql="select * from userinfo";
                       query.exec(select_sql);
                       while(query.next()){
                       QString u=query.value(0).toString();
                       if(u!="1%"){retUsrName+=u;
                                   retUsrName+='#';}
                          }
                       retUsrName+='\0';
                       select_sql="select * from userinfo";
                       query.exec(select_sql);
                       int PassWord=0;
                       while(query.next()){
                           QString u=query.value(0).toString();
                           QString p=query.value(1).toString();
                           QString lixi=query.value(3).toString();
                           if(str1==u&&str2==p)
                           {
                               QString update_sql = "update userinfo set ipaddr = :ipaddr where name = :name";
                               query.prepare(update_sql);
                               query.bindValue(":ipaddr", ipadd);
                               query.bindValue(":name", u);
                               if(!query.exec())
                               {
                                   qDebug() << query.lastError();
                               }
                               else
                               {
                                   qDebug()<<"success";
                               }
                               update_sql = "update userinfo set status = :status where name = :name";
                               query.prepare(update_sql);
                               query.bindValue(":status","1");
                               query.bindValue(":name", u);
                               if(!query.exec())
                               {
                                   qDebug() << query.lastError();
                               }
                               else
                               {
                                   qDebug()<<"success";
                               }//更新状态

                               update_sql = "update userinfo set lixian = :lixian where name = :name";
                               query.prepare(update_sql);
                               query.bindValue(":lixian"," ");
                               query.bindValue(":name", u);
                               if(!query.exec())
                               {
                                   qDebug() << query.lastError();
                               }
                               else
                               {
                                   qDebug()<<"success";
                               }//更新离线消息

                               strcpy(buf,"1");
                               send(sockClient,buf, sizeof(buf), 0);
                               emit q->m->valuechanged(u+"login");
                               Sleep(100);
                               QByteArray bufByte= retUsrName.toLatin1();
                               char *buff=bufByte.data();
                               strcpy(buf,buff);
                               send(sockClient,buf,sizeof(buf), 0);
                               emit q->m->valuechanged(retUsrName);
                               Sleep(100);
                               bufByte= lixi.toLatin1();
                               buff=bufByte.data();
                               strcpy(buf,buff);
                               send(sockClient,buf,sizeof(buf), 0);
                               emit q->m->valuechanged(lixi);
                               retUsrName="";                              
                               PassWord=1;
                           }
                       }
                       if(PassWord==0)
                       {
                           strcpy(buf,"-1");
                           send(sockClient,buf, sizeof(buf), 0);
                       }
                       }break;//登录

            case 1 :{
                int flag=0;
                recv(sockClient, buf, sizeof(buf), 0);
                str1= QString(QLatin1String(buf));
                emit q->m->valuechanged(str1);//account

                recv(sockClient, buf, sizeof(buf), 0);
                str2= QString(QLatin1String(buf));
                emit q->m->valuechanged(str2);//password

                recv(sockClient, buf, sizeof(buf), 0);
                str3= QString(QLatin1String(buf));
                emit q->m->valuechanged(str3);//telephone number

                QString select_sql="select * from userinfo";
                query.exec(select_sql);
                while(query.next()){
                    QString u=query.value(0).toString();
                    if(str1==u){ strcpy(buf,"-1"); flag=1;}
                }
                if(flag!=1){
                    QString lixian="";
                    QString st="0";
                    QString insert_sql=QString("insert into userinfo values('%1','%2','%3','%4','%5','%6')").arg(str1).arg(str2).arg(ipadd).arg(lixian).arg(st).arg(str3);
                    if(query.exec(insert_sql))
                    {

                        strcpy(buf,"1");
                        emit q->m->valuechanged("zhuce success");
                    }
                    else emit q->m->valuechanged("zhuce fail");
                }
                send(sockClient,buf, sizeof(buf), 0);
                }break;//注册
            case 2 :
            {
                recv(sockClient, buf, sizeof(buf), 0);//接收用户名
                str1= QString(QLatin1String(buf));
                emit q->m->valuechanged(str1);
                recv(sockClient, buf, sizeof(buf), 0);//接收电话号码
                str2= QString(QLatin1String(buf));
                emit q->m->valuechanged(str2);
                QString select_sql="select * from userinfo";
                query.exec(select_sql);
                int TELE=0;
                while(query.next()){
                    QString u=query.value(0).toString();
                    QString p=query.value(1).toString();
                    QString tele=query.value(5).toString();
                    if(str1==u&&str2==tele)
                    {
                        p+='\0';
                        QByteArray bufByte= p.toLatin1();
                        char *buff=bufByte.data();
                        memset(buf, 0, sizeof(buf));
                        strcpy(buf,buff);
                        TELE=1;
                        emit q->m->valuechanged("zhaohui success");
                    }
                }
                if(TELE==0) strcpy(buf,"-1");
                send(sockClient,buf,sizeof(buf), 0);
                memset(buf, 0, sizeof(buf));
            }break;//找回密码
/*********************************************************************************************************************/
            case 3 :
            {
                recv(sockClient, buf, sizeof(buf), 0);
                str1= QString(QLatin1String(buf));
                emit q->m->valuechanged(str1);
                QString select_sql="select * from userinfo";
                query.exec(select_sql);
                while(query.next()){
                    QString u=query.value(0).toString();
                    QString ipaddrs=query.value(2).toString();
                    QString statu=query.value(4).toString();
                    if(str1==u)
                    {
                        ipaddrs+='\0';
                        QByteArray bufByte= ipaddrs.toLatin1();
                        char *buff=bufByte.data();
                        memset(buf, 0, sizeof(buf));
                        strcpy(buf,buff);
                        send(sockClient,buf,sizeof(buf), 0);
                        memset(buf, 0, sizeof(buf));
                        emit q->m->valuechanged(ipaddrs);
Sleep(100);

                        bufByte=statu.toLatin1();
                        buff=bufByte.data();
                        strcpy(buf,buff);
                        send(sockClient,buf,sizeof(buf), 0);
                        emit q->m->valuechanged(statu);
                    }
                }
            }break;//获取离线与在线状态
            case 4 :
            {
                recv(sockClient, buf, sizeof(buf), 0);
                str1= QString(QLatin1String(buf));
                emit q->m->valuechanged(str1);
                QString select_sql="select * from userinfo";
                query.exec(select_sql);
                while(query.next()){
                    QString u=query.value(0).toString();
                    QString lixianmes=query.value(3).toString();
                    if(str1==u)
                    {
                        recv(sockClient, buf, sizeof(buf), 0);
                        str2= QString(QLatin1String(buf));                        
                        lixianmes+=str2;
                        lixianmes+="# ";
                        QString update_sql = "update userinfo set lixian = :lixian where name = :name";
                        query.prepare(update_sql);
                        query.bindValue(":lixian",lixianmes);
                        query.bindValue(":name",u);
                        if(!query.exec())
                        {
                            qDebug() << query.lastError();
                        }
                        else
                        {
                            qDebug()<<"success";
                        }

                        strcpy(buf,"1");
                        send(sockClient,buf,sizeof(buf), 0);

                        emit q->m->valuechanged(buf);
                    }
                }
            }break;//离线聊天
            case 5 : {
                       recv(sockClient, buf, sizeof(buf), 0);
                       str1= QString(QLatin1String(buf));
                       emit q->m->valuechanged(str1);
                       QString update_sql = "update userinfo set status = :status where name = :name";
                       query.prepare(update_sql);
                       query.bindValue(":status","0");
                       query.bindValue(":name",str1);
                       if(!query.exec())
                       {
                           qDebug() << query.lastError();
                       }
                       else
                       {
                           qDebug()<<"success";
                       }
            }//下线
            default: ;break;
            }
            //str2= QString(QLatin1String(buf));
            //emit q->m->valuechanged(str2);
            //ret = send(sockClient,buf, strlen(buf), 0);

            //if (SOCKET_ERROR == ret)
            //{
              //  printf("socket send failed\n");
               // closesocket(sockClient);
              //  return -1;
           // }
        }
    }
    database.close();
    closesocket(sockClient);
    return 0;
}
DWORD WINAPI  AcceptProc(LPARAM lparam)
{
    par2 *p=new par2;
    p->m=((par *)lparam)->m;
    p->s=((par*)lparam)->s;
    SOCKET SerSock=p->s;
    sockaddr_in clientAddr;
    int nAddrLen = sizeof(sockaddr);
    while (1)
    {
        //新建一个socket，用于客户端
        SOCKET *sClient = new SOCKET;
        //等待客户端的连接
        *sClient= accept(SerSock, (sockaddr*)&clientAddr, &nAddrLen);
        p->clientAddr=clientAddr;
        p->s=*sClient;
        if (INVALID_SOCKET == *sClient)
        {
            printf("socket accept failed\n");
            WSACleanup();
            closesocket(SerSock);
            delete sClient;
            return 0;
        }
        //创建线程为客户端做数据收发
        CreateThread(0, 0, (LPTHREAD_START_ROUTINE)clientProc, (LPVOID*)p, 0, 0);
    }
    closesocket(SerSock);
    WSACleanup();
    return 0;
}
