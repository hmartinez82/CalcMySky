#ifndef INCLUDE_ONCE_C49956E1_F7B6_4759_8745_711BBDFE6FE7
#define INCLUDE_ONCE_C49956E1_F7B6_4759_8745_711BBDFE6FE7

#include <string>
#include <QVector4D>
#include <QOpenGLFunctions_3_3_Core>
#include <glm/glm.hpp>

extern QOpenGLFunctions_3_3_Core gl;

struct MustQuit{};

inline QVector4D QVec(glm::vec4 v) { return QVector4D(v.x, v.y, v.z, v.w); }
inline QString toString(int x) { return QString::number(x); }
inline QString toString(double x) { return QString::number(x, 'g', 17); }
inline QString toString(float x) { return QString::number(x, 'g', 9); }
inline QString toString(glm::vec2 v) { return QString("vec2(%1,%2)").arg(double(v.x), 0,'g',9)
                                                             .arg(double(v.y), 0,'g',9); }
inline QString toString(glm::vec4 v) { return QString("vec4(%1,%2,%3,%4)").arg(double(v.x), 0,'g',9)
                                                                   .arg(double(v.y), 0,'g',9)
                                                                   .arg(double(v.z), 0,'g',9)
                                                                   .arg(double(v.w), 0,'g',9); }
void renderUntexturedQuad();
void checkFramebufferStatus(const char*const fboDescription);
void qtMessageHandler(const QtMsgType type, QMessageLogContext const&, QString const& message);

// Function useful only for debugging
void dumpActiveUniforms(const GLuint program);

#endif
