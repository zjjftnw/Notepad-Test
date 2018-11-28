#ifndef CUSARROW_H
#define CUSARROW_H

#include <QGraphicsItem>
#include <QGraphicsRectItem>
#include <memory>

class CusRect;

class CusArrow:public QGraphicsItem
{
//    Q_OBJECT
public:
    CusArrow(int,std::shared_ptr<CusRect> ,std::shared_ptr<CusRect> ,QGraphicsItem *parent = nullptr);
    ~CusArrow() override;
    QRectF boundingRect() const override;
    void adjust();

    int getFlag()const{return flag;}
private:
    void paint(QPainter *, const QStyleOptionGraphicsItem* ,QWidget *) override;

    int flag;//对应 mazeoption 中的id
    QPointF arrowP1;//箭头张开的点
    QPointF arrowP2;//箭头张开的点
    QPointF p1;//箭头的起点位置
    QPointF p2;//箭头的终点位置
    std::shared_ptr<CusRect> rectA;//起点的方框
    std::shared_ptr<CusRect> rectB;//终点的方框
};

#endif // CUSARROW_H
