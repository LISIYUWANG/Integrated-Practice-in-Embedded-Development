#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{

    ui->setupUi(this);
    speed=new QButtonGroup(this);            //分组
    method=new QButtonGroup(this);            //分组
    speed->addButton(ui->s1,1);
    speed->addButton(ui->s2,5);
    speed->addButton(ui->s3,9);
    speed->addButton(ui->s4,10);

    method->addButton(ui->m1,2);
    method->addButton(ui->m2,3);
    method->addButton(ui->m3,1);
    method->addButton(ui->m4,0);

    ui->s1->setChecked(1);
    ui->m1->setChecked(1);

    ui->adTable->clearContents();
    ui->adTable->setRowCount(0);
    ui->adTable->setColumnCount(1);
    ui->adTable->setHorizontalHeaderLabels(QStringList()<<QString::fromUtf8("广告"));



    //自动刷新
    ifstream in("C:\\Users\\Administrator\\Desktop\\myclient\\ad.txt");
    string line;
    if(in)
    {
        while(getline(in,line))
        {
            //按行
            qDebug()<<QString::fromStdString(line)<<endl;
            int RowCont;
            RowCont = ui->adTable->rowCount();
            ui->adTable->insertRow(RowCont);
            ui->adTable->setItem(RowCont,0,new QTableWidgetItem(QString::fromStdString(line)));
        }
    }
    else
    {
        qDebug()<<"No ad file"<<endl;
    }

    //UI设定IP
    ui->ipText->setText("192.168.24.18");
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_refreshButton_clicked()
{
//    FILE *fp = NULL;
//    char tmp[100];
//    fp = fopen("test.txt", "r");
//    while(!feof(fp))
//    {
//    	fscanf(fp, "%s\r\n", tmp);
//    	printf("%s\n", tmp);
//    }
//    fclose(fp);
    ui->adTable->clearContents();
    ui->adTable->setRowCount(0);
    ui->adTable->setColumnCount(1);
    ui->adTable->setHorizontalHeaderLabels(QStringList()<<QString::fromUtf8("广告"));

    ifstream in("C:\\Users\\Administrator\\Desktop\\myclient\\ad.txt");
    string line;
    if(in)
    {
        while(getline(in,line))
        {
            //按行
            qDebug()<<QString::fromStdString(line)<<endl;
            int RowCont;
            RowCont = ui->adTable->rowCount();
            ui->adTable->insertRow(RowCont);
            ui->adTable->setItem(RowCont,0,new QTableWidgetItem(QString::fromStdString(line)));
        }
    }
    else
    {
        qDebug()<<"No ad file"<<endl;
    }

    saveAd();
}

void MainWindow::on_pushButton_clicked()
{
    fstream f("C:\\Users\\Administrator\\Desktop\\myclient\\ad.txt");
    string line;
    line = ui->adText->toPlainText().toStdString();
    if(f)
    {
        f<<line;
        qDebug()<<"add Success!"<<endl;
    }
    else
    {
        qDebug()<<"No add ad file"<<endl;
    }
    f.close();
}

void MainWindow::on_addAd_clicked()
{
    fstream f("C:\\Users\\Administrator\\Desktop\\myclient\\ad.txt",ios::out|ios::app);
    string line;
    line = ui->adText->toPlainText().toStdString();
    if(f)
    {
        f<<line;
        qDebug()<<"add Success!"<<endl;
        // 写入文件成功 现在需要刷新table
        int RowCont;
        RowCont = ui->adTable->rowCount();
        ui->adTable->insertRow(RowCont);
        ui->adTable->setItem(RowCont,0,new QTableWidgetItem(QString::fromStdString(line)));
    }
    else
    {
        qDebug()<<"No add ad file"<<endl;
    }
    f.close();
    //关闭后 清空文本
    ui->adText->clear();
}

void MainWindow::on_pushButton_3_clicked()
{
    int rowNum;
    rowNum = ui->adTable->currentRow();
    QList<QTableWidgetItem*> items = ui->adTable->selectedItems();
    int count = items.count();
    for(int i = 0; i < count; i++)
    {
        int row = ui->adTable->row(items.at(i));
        QTableWidgetItem *item = items.at(i);
        QString text = item->text(); //获取内容
        qDebug()<<text<<endl;
        qDebug()<<rowNum<<endl;
    }
    ui->adTable->removeRow(rowNum);
    // 删除之后要保存到文件中
    saveAd();
}

void MainWindow::saveAd()
{

    fstream f("C:\\Users\\Administrator\\Desktop\\myclient\\ad.txt",ios::out);
    if(f)
    {
        int romCount = ui->adTable->rowCount();//获取总行数
        for(int i=0;i<romCount;i++)//行
        {
             QString rowstring;
             for(int j=0;j<1;j++)//列
             {
                  //遍历表格中的字符串
                  rowstring +=ui->adTable->item(i,j)->text();
              }
              //rowstring += "\n";//下一行空格
               qDebug()<<rowstring<<endl;
               f<<rowstring.toStdString()<<endl;
              //out<<rowstring;   //把每行数据输入文件对象
          }
    }
    else
    {
        qDebug()<<"No add ad file"<<endl;
    }

}

void MainWindow::on_runButton_clicked()
{
        int port = 25555;
        QString ip = ui->ipText->text();
        QString mtd = QString::number(method->checkedId());
        QString spd = QString::number(speed->checkedId());
        QString text ;
        // qDebug()<<ip<<endl<<mtd<<endl<<spd<<endl<<endl;

        //get selected
        QList<QTableWidgetItem*> items = ui->adTable->selectedItems();
        if(items.empty())
        {
            QMessageBox::information(this, tr("提示"),tr("请选择广告语！"), QMessageBox::Ok);
           qDebug("No Choose");
        }
        else
        {
            int count = items.count();
            for(int i = 0; i < count; i++)
            {
                int row = ui->adTable->row(items.at(i));
                QTableWidgetItem *item = items.at(i);
                text = item->text();
                qDebug()<<text<<endl;
            }
            qDebug()<<ip<<endl<<mtd<<endl<<spd<<endl<<text<<endl;
            QString send = mtd+'$'+spd+"$1"+'$'+text;
            qDebug()<<"send here"<<send<<endl;


            //begin send
            QTcpSocket *socket = new QTcpSocket(this);
            qDebug()<<ip<<endl;
            socket->connectToHost(QHostAddress(ip),25555);
            if(socket->waitForConnected())
            {
                QByteArray block = send.toUtf8();
                qDebug()<<"send num"<<socket->write(block)<<endl;
                socket->flush();
                qDebug()<<"send:"<<send<<endl;
                qDebug()<<"send sucess"<<endl;
                QMessageBox::information(this, tr("提示"),tr("发送成功！"), QMessageBox::Ok);
            }
            socket->abort();
        }


}

void MainWindow::on_resetButton_clicked()
{
    int port = 25555;
    QString ip = ui->ipText->text();
    QString mtd = "reset";
    QString spd = "reset";
    QString text = "reset";
    // qDebug()<<ip<<endl<<mtd<<endl<<spd<<endl<<endl;

    qDebug()<<ip<<endl<<mtd<<endl<<spd<<endl<<text<<endl;
    QString send = mtd+'$'+spd+"$1"+'$'+text;
    qDebug()<<"send here"<<send<<endl;


        //begin send
    QTcpSocket *socket = new QTcpSocket(this);
    qDebug()<<ip<<endl;
    socket->connectToHost(QHostAddress(ip),25555);
    if(socket->waitForConnected())
    {
        QByteArray block = send.toUtf8();
        qDebug()<<"send num"<<socket->write(block)<<endl;
        socket->flush();
        qDebug()<<"send:"<<send<<endl;
        qDebug()<<"send sucess"<<endl;
        QMessageBox::information(this, tr("提示"),tr("复位成功！"), QMessageBox::Ok);
     }
     socket->abort();

     //删文件
     fstream f("C:\\Users\\Administrator\\Desktop\\myclient\\ad.txt",ios::out);
     f.close();
     //刷新
     ui->adTable->clearContents();
     ui->adTable->setRowCount(0);
     ui->adTable->setColumnCount(1);
     ui->adTable->setHorizontalHeaderLabels(QStringList()<<QString::fromUtf8("广告"));

     ifstream in("C:\\Users\\Administrator\\Desktop\\myclient\\ad.txt");
     string line;
     if(in)
     {
         while(getline(in,line))
         {
             //按行
             qDebug()<<QString::fromStdString(line)<<endl;
             int RowCont;
             RowCont = ui->adTable->rowCount();
             ui->adTable->insertRow(RowCont);
             ui->adTable->setItem(RowCont,0,new QTableWidgetItem(QString::fromStdString(line)));
         }
     }
     else
     {
         qDebug()<<"No ad file"<<endl;
     }

     saveAd();
}
