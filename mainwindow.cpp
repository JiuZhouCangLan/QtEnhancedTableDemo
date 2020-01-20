#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "enhancedheader.h"
#include "enhancedtableview.h"
#include "enhancedstandarditemmodel.h"
#include <QtDebug>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    /*
     * initiate some data
     */
    EnhancedTableView *tableView = ui->tableView;
    EnhancedStandardItemModel *model = new EnhancedStandardItemModel(4, 5, this);
    tableView->setModel(model);
    QStringList strList = QString("Once you have installed Qt you can "
                                  "start Qt Assistant in the same way "
                                  "as any other application on the development "
                                  "host The Qt Assistant main window contains as"
                                  "idebar (1) with navigation windows for:").split(" ");
    for(int i = 0; i < 5; i++) {
        model->setHeaderData(i, Qt::Horizontal, strList.takeFirst().append(" ").repeated(5));
        for (int j = 0; j < 4; j++) {
            model->setData(model->index(j, i), strList.takeFirst());
        }
    }


    /*
     * filter feature,default enable
    */
    tableView->setShowFilters(true);

    /*
     * HorizontalHeader with Text WordWrap feature,default enable
    */
    tableView->setHorizontalHeaderWrap(true);

    /*
     * checkBox feature,set cell(1,1) to checkBox
    */
    model->setCellIsCheckable(model->index(1, 1));
    model->setData(model->index(1, 1), Qt::Checked, Qt::CheckStateRole); // set checked
    int state = model->data(model->index(1, 1), Qt::CheckStateRole).toInt(); // get check state
    qDebug() << static_cast<Qt::CheckState>(state);

    /*
     * html feature,set html to cell(2,2)
    */
    model->setData(model->index(2, 2), "<a href='github.com'>this is a link</a>",
                   EnhancedStandardItemModel::HtmlRole);
    connect(tableView, &EnhancedTableView::linkActivated, this, [ = ](QString link) {
        qDebug() << link;
    });
}

MainWindow::~MainWindow()
{
    delete ui;
}
