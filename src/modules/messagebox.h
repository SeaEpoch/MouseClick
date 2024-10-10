#ifndef MESSAGEBOX_H
#define MESSAGEBOX_H

#include <QMessageBox>

class MessageBox : public QMessageBox
{
    Q_OBJECT
public:
    explicit MessageBox(QWidget* parent = nullptr);
    ~MessageBox();

    void setIcon(Icon icon);

private:
    Q_DISABLE_COPY(MessageBox)
};

#endif // MESSAGEBOX_H
