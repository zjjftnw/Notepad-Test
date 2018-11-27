#include "excelres.h"
#include "globalfunc.h"
#include <QFile>
#include <QMessageBox>

namespace  {
QMap<EXCEL_FILES, QString> FileMap{
    {EXCEL_FILES::pot, "pot"},
    {EXCEL_FILES::mazeStage, "mazeStage"},
    {EXCEL_FILES::mazeOption, "mazeOption"},
    {EXCEL_FILES::item, "item"}
};
}

ExcelRes::ExcelRes()
    :ExcelBase ()
{

}

bool ExcelRes::openExcel(EXCEL_FILES file, bool isBackup)
{
    QString relative = isBackup?  myfunc::getBackupExcelFilesPath(): myfunc::getExcelFilesPath();
    QString fullPath = relative + "/" +  FileMap[file] + ".xlsx";
    if(ExcelBase::open(fullPath)){
        initFields();
    }
    return true;
}

void ExcelRes::closeExcel(){
    close();
}

bool ExcelRes::addFields(const QVector<std::tuple<QString,FIELD_TYPE>> &fields )
{
    for(auto it = fields.cbegin(); it != fields.cend();++it){
        QString fieldname = std::get<0>(*it);
        auto fieldtype = std::get<1>(*it);
        QString qs;
        switch (fieldtype) {
        case FIELD_TYPE::NON: qs = "";break;
        case FIELD_TYPE::INT:  qs = "INT";break;
        case FIELD_TYPE::STR: qs = "STR";break;
        case FIELD_TYPE::LIST: qs = "LIST";break;
        }
        QVariant qvar(qs);
        auto col = it - fields.cbegin() + 1;
        write(1, col, qvar);
        write(2,col, QVariant("字段描述"));
        write(3, col, QVariant(fieldname));
    }
    return true;
}

ExcelData& ExcelRes::at(int id)
{
    auto it = m_map.find(id);
    if(it == m_map.end()){
        qFatal("excel: %s 中不存在 id: %d 的数据",getFullPath().toStdString().c_str(), id);
    }
    return *(it.value());
}

ExcelData ExcelRes::get(int id)
{
    return at(id);
}

bool ExcelRes::set(int id, ExcelData *pdata)
{
    auto it = m_map.find(id);
    if(it != m_map.end()){
        it.value().reset(pdata);
    }else{
        m_map.insert(id, std::shared_ptr<ExcelData>(pdata));
    }
    return true;
}

bool ExcelRes::saveExcel()
{
    int row = 3;
    for(auto it = m_map.cbegin();it != m_map.cend();++it){
        auto list = toExcel(*( it.value()));
        write(++row,  list);
    }
    save();
    return true;
}

void ExcelRes::loadExcel()
{
    m_map.clear();
    QList<QList<QVariant>> cells;
    readAll(cells);
    for(auto it_row = cells.cbegin(); it_row != cells.cend();++it_row){
        auto row = it_row - cells.cbegin() + 1;
        if( row < 4){
            continue;
        }
        auto vars = toObject(*it_row);
        m_map.insert(std::get<0>(vars), std::get<1>(vars));
    }
}


bool ExMazeOption::initFields()
{
    addFields({
                  std::make_tuple("id",FIELD_TYPE::INT),
                  std::make_tuple("text",FIELD_TYPE::STR),
                  std::make_tuple("activecond",FIELD_TYPE::STR),
                  std::make_tuple("disabledTips",FIELD_TYPE::STR),
                  std::make_tuple("linkStageId",FIELD_TYPE::INT),
                  std::make_tuple("events",FIELD_TYPE::LIST),
                  std::make_tuple("isonlyonce",FIELD_TYPE::INT)
              });
    return true;
}

QList<QVariant> ExMazeOption::toExcel(const ExcelData &data)
{
    auto &option = static_cast<const MazeOption&>(data);
    QList<QVariant> list({
                             QVariant(option.id),
                             QVariant(option.text),
                             QVariant(option.activecond),
                             QVariant(option.disabledTips),
                             QVariant(option.linkStageId),
                             QVariant(option.events.join(";")),
                             QVariant(option.isonlyonce?1:0)
                         });
    return list;
}

std::tuple<int,std::shared_ptr<ExcelData>> ExMazeOption::toObject(const QList<QVariant>&list)
{
    MazeOption option;
    for(auto it = list.cbegin(); it !=  list.cend();++it){
        auto col = it -  list.cbegin() + 1;
        switch (col) {
        case 1:  option.id = it->toInt() ; break;
        case 2:  option.text = it->toString() ; break;
        case 3:  option.activecond = it->toString() ; break;
        case 4:  option.disabledTips = it->toString() ; break;
        case 5:  option.linkStageId = it->toInt() ; break;
        case 6:option.events = it->toString().split(";");break;
        case 7:  option.isonlyonce = it->toInt(); break;
        }
    }
    return std::make_tuple(option.id, std::shared_ptr<ExcelData>(new MazeOption(option)));
}

std::tuple<int,std::shared_ptr<ExcelData>> ExItem::toObject(const QList<QVariant>&list)
{
    Item item;
    for(auto it = list.cbegin(); it !=  list.cend();++it){
        auto col = it -  list.cbegin() + 1;
        switch (col) {
        case 1:  item.id = it->toInt() ; break;
        case 6: item.type = it->toInt();break;
        case 19: item.remark = it->toString();break;
        }
    }
    return std::make_tuple(item.id, std::shared_ptr<ExcelData>(new Item(item)));
}

QVector<Item> ExItem::getValidTickets()
{
    QVector<Item> vec;
    auto container = getContainer();
    if(container.size() != 0){
        for(auto it = container.cbegin();it != container.cend();++it){
            auto &item = static_cast<Item&>(*(it.value()));
            if(item.type == 1 || item.type == 2 || item.type == 4){
                vec.append(item);
            }
        }
    }
    return vec;
}

QList<QVariant>ExPot::toExcel(const ExcelData &data)
{
    auto &option = static_cast<const Pot&>(data);
    QList<QVariant> list({
                             QVariant(option.id),
                             QVariant(option.name),
                             QVariant(option.desc),
                             QVariant(option.type),
                             QVariant(option.x),
                             QVariant(option.y),
                             QVariant(option.icon),
                             QVariant(option.icon1),
                             QVariant(option.unlock.join(";")),
                             QVariant(option.cond),
                             QVariant(option.stage.join(";")),
                             QVariant(option.target),
                             QVariant(option.weight.join(";"))
                         });
    return list;
}

std::tuple<int,std::shared_ptr<ExcelData>>ExPot::toObject(const QList<QVariant>&list)
{
    Pot option;
    for(auto it = list.cbegin(); it !=  list.cend();++it){
        auto col = it -  list.cbegin() + 1;
        switch (col) {
        case 1:  option.id = it->toInt() ; break;
        case 2:  option.name = it->toString() ; break;
        case 3:  option.desc = it->toString() ; break;
        case 4:  option.type = it->toInt() ; break;
        case 5:  option.x = it->toInt() ; break;
        case 6:option.y = it->toInt();break;
        case 7:  option.icon = it->toString(); break;
        case 8:  option.icon1 = it->toString(); break;
        case 9:  option.unlock = it->toString().split(";"); break;
        case 10:  option.cond = it->toInt(); break;
        case 11:  option.stage = it->toString().split(";"); break;
        case 12:  option.target = it->toInt(); break;
        case 13:  option.weight = it->toString().split(";"); break;
        }
    }
    return std::make_tuple(option.id, std::shared_ptr<ExcelData>(new Pot(option)));
}
