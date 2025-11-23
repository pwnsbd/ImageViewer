#ifndef IMAGEPROPERTY_H
#define IMAGEPROPERTY_H

#include <QString>

enum class PropertyId{
    Brightness,
    Contrast,
};

class ImageProperty{

public:
    ImageProperty(PropertyId id,
                  const QString& name,
                  int minValue,
                  int maxValue,
                  int initialValue
                  ) : m_id(id), m_name(name), m_min(minValue), m_max(maxValue), m_initial(initialValue) {}

PropertyId id() const {return m_id;}
    const QString& name() const {return m_name;}
int min() const {return m_min;}
    int max() const {return m_max;}
int value() const {return m_initial;}

void setValue(int value)
{
    if (value < m_min) value = m_min;
    if (value > m_max) value = m_max;
    m_initial = value;
}

private:
    PropertyId m_id;
    QString m_name;
    int m_min;
    int m_max;
    int m_initial;
};

#endif // IMAGEPROPERTY_H
