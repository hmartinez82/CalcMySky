#include "GLWidget.hpp"
#include <chrono>
#include <QMouseEvent>
#include <QMessageBox>
#include <QSurfaceFormat>
#include "../common/util.hpp"
#include "util.hpp"
#include "ToolsWidget.hpp"

GLWidget::GLWidget(QString const& pathToData, AtmosphereRenderer::Parameters const& params, ToolsWidget* tools, QWidget* parent)
    : QOpenGLWidget(parent)
    , params(params)
    , pathToData(pathToData)
    , tools(tools)
{
    connect(this, &GLWidget::frameFinished, tools, &ToolsWidget::showFrameRate);
}

GLWidget::~GLWidget()
{
    // Let the destructor of renderer have current GL context. This avoids warnings from QOpenGLTexturePrivate::destroy().
    makeCurrent();
}

void GLWidget::initializeGL()
{
    if(!initializeOpenGLFunctions())
    {
        throw InitializationError{tr("Failed to initialize OpenGL %1.%2 functions")
                                    .arg(QSurfaceFormat::defaultFormat().majorVersion())
                                    .arg(QSurfaceFormat::defaultFormat().minorVersion())};
    }

    renderer.reset(new AtmosphereRenderer(*this,pathToData,params,tools));
    tools->updateParameters(params);
    const auto update=qOverload<>(&GLWidget::update);
    connect(renderer.get(), &AtmosphereRenderer::needRedraw, this, update);
    connect(renderer.get(), &AtmosphereRenderer::loadProgress, this, &GLWidget::onLoadProgress);
    connect(tools, &ToolsWidget::settingChanged, this, update);
    connect(tools, &ToolsWidget::setScattererEnabled, renderer.get(), &AtmosphereRenderer::setScattererEnabled);
    connect(tools, &ToolsWidget::reloadShadersClicked, this, &GLWidget::reloadShaders);
    try
    {
        renderer->loadData();
    }
    catch(Error const& ex)
    {
        QMessageBox::critical(this, ex.errorType(), ex.what());
    }
}

void GLWidget::onLoadProgress(QString const& currentActivity, const int stepsDone, const int stepsToDo)
{
    tools->onLoadProgress(currentActivity,stepsDone,stepsToDo);
    // Processing of load progress has likely drawn something on some widgets,
    // which would take away OpenGL context, so we must take it back.
    makeCurrent();
}

void GLWidget::paintGL()
{
    if(!isVisible()) return;
    if(!renderer->readyToRender()) return;

    const auto t0=std::chrono::steady_clock::now();
    renderer->draw();
    glFinish();
    const auto t1=std::chrono::steady_clock::now();
    emit frameFinished(std::chrono::duration_cast<std::chrono::microseconds>(t1-t0).count());

    if(lastRadianceCapturePosition.x()>=0 && lastRadianceCapturePosition.y()>=0)
        updateSpectralRadiance(lastRadianceCapturePosition);
}

void GLWidget::resizeGL(int w, int h)
{
    renderer->resizeEvent(w,h);
}

void GLWidget::updateSpectralRadiance(QPoint const& pixelPos)
{
    makeCurrent();
    if(const auto spectrum=renderer->getPixelSpectralRadiance(pixelPos); !spectrum.empty())
    {
        if(tools->handleSpectralRadiance(spectrum))
            lastRadianceCapturePosition=pixelPos;
    }
}

void GLWidget::mouseMoveEvent(QMouseEvent* event)
{
    if(event->buttons()==Qt::LeftButton && !(event->modifiers() & (Qt::ControlModifier|Qt::ShiftModifier)))
    {
        updateSpectralRadiance(event->pos());
        return;
    }

    renderer->mouseMove(event->x(), event->y());
    update();
}

void GLWidget::mousePressEvent(QMouseEvent* event)
{
    if(event->buttons()==Qt::LeftButton && !(event->modifiers() & (Qt::ControlModifier|Qt::ShiftModifier)))
    {
        updateSpectralRadiance(event->pos());
        return;
    }

    if(event->modifiers() & Qt::ControlModifier)
        renderer->setDragMode(AtmosphereRenderer::DragMode::Sun, event->x(), event->y());
    else
        renderer->setDragMode(AtmosphereRenderer::DragMode::Camera, event->x(), event->y());
}

void GLWidget::mouseReleaseEvent(QMouseEvent*)
{
    renderer->setDragMode(AtmosphereRenderer::DragMode::None);
}

void GLWidget::reloadShaders()
{
    makeCurrent();
    renderer->reloadShaders();
    update();
}
