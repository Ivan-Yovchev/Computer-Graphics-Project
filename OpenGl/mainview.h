#ifndef MAINVIEW_H
#define MAINVIEW_H

#include "model.h"

#include <QKeyEvent>
#include <QMouseEvent>
#include <QOpenGLWidget>
#include <QOpenGLFunctions_3_3_Core>
#include <QOpenGLDebugLogger>
#include <QOpenGLShaderProgram>
#include <QTimer>
#include <QVector>
#include <QVector3D>
#include <QMatrix4x4>
#include <QImage>
#include <memory>
#include <vector>

class MainView : public QOpenGLWidget, protected QOpenGLFunctions_3_3_Core {
    Q_OBJECT

public:
    enum ShadingMode : GLuint
    {
        PHONG = 0, NORMAL, GOURAUD
    };

private:
    QOpenGLDebugLogger *debugLogger;
    QTimer timer; // timer used for animation

    QOpenGLShaderProgram shaderNormalProgram;
    QOpenGLShaderProgram shaderGouraudProgram;
    QOpenGLShaderProgram shaderPhongProgram;
    QOpenGLShaderProgram* shaderActiveProgram;
    ShadingMode activeMode;
//    GLint uniformModelViewTransform;
    GLint uniformProjectionTransform;
//    GLint uniformNormalTransform;

    struct Illumination {
        GLfloat emissionFactor;
        GLfloat ambientFactor;
        GLfloat diffuseFactor;
        GLfloat specularFactor;
        GLfloat specularExponent;
    };

    struct AnimationState3D {
        QVector3D baseValue;
        QVector3D currentValue;
        QVector3D deltaValue;
    };

    struct Mesh {
        std::string name;
        GLuint vertexArrayObject;
        GLuint vertexBufferObject;
        GLuint indexBufferObject;
        GLuint surfaceTexture;
        GLuint normalTexture;
        bool useNormalMap = false;
        GLuint numIndices;
        Illumination illumination;
        struct {
            struct {
                QVector3D translation;
                QVector3D rotation;
                float scale = 1.0f;
            } base;
            struct {
                AnimationState3D state;
            } rotation;
            struct {
                // TODO: to be replaced by std::variant<QVector3D, std::shared_ptr<Mesh>>
                // TODO: However, for the CG course we stick to C++14, as we do not know if
                // TODO: this is going to be build with a C++17 capable compiler.
                QVector3D pivot;
                std::shared_ptr<const Mesh> pivotObject;
                AnimationState3D state;
            } orbit;
        } transform;
    };

    std::vector<std::shared_ptr<Mesh>> objects;
    Illumination planetIllumination;
    Illumination sunIllumination;
    Illumination horseIllumination;
    Illumination universeIllumination;

    QVector3D universeCenter;
    std::shared_ptr<Mesh> cameraLock;
    QMatrix4x4 viewTransform;
    QMatrix4x4 projectionTransform;

    struct {
        QVector3D position;
        QVector3D color;
    } light;

public:
    MainView(QWidget *parent = 0);
    ~MainView();

    // Functions for widget input events
    void setRotation(int rotateX, int rotateY, int rotateZ);
    void setScale(int scale);
    void setShadingMode(ShadingMode shading);
    void setCameraLock(std::size_t index);

protected:
    void initializeGL();
    void resizeGL(int newWidth, int newHeight);
    void paintGL();
    void applyUniforms(const std::shared_ptr<Mesh>&);

    // Functions for keyboard input events
    void keyPressEvent(QKeyEvent *ev);
    void keyReleaseEvent(QKeyEvent *ev);

    // Function for mouse input events
    void mouseDoubleClickEvent(QMouseEvent *ev);
    void mouseMoveEvent(QMouseEvent *ev);
    void mousePressEvent(QMouseEvent *ev);
    void mouseReleaseEvent(QMouseEvent *ev);
    void wheelEvent(QWheelEvent *ev);

    std::vector<quint8> imageToBytes(QImage);
    void loadTexture(QString file, GLuint texturePtr);

    std::shared_ptr<Mesh> loadMesh(QString objFile, QString surfaceTexture, Illumination illumination);
    std::shared_ptr<Mesh> loadMesh(QString objFile, QString surfaceTexture, QString normalTexture, Illumination illumination);
    void destroyMesh(std::shared_ptr<Mesh>&);
    QMatrix4x4 getMeshTransform(const std::shared_ptr<const Mesh>& mesh) const;

protected:
    typedef struct {
        QVector3D rotation = QVector3D(0, 0, 0);
        GLfloat scale = 100.0;
    } State;

    typedef struct {
        int dragType;
        int initialX, initialY;
        QVector3D initialRotation;
    } Drag;

    State m_state;
    Drag m_drag;

    bool inRotationUpdate, inScaleUpdate;

    void animate();

private slots:
    void onMessageLogged( QOpenGLDebugMessage Message );

    void updateProjectionTransform();
    void updateViewTransform();

signals:
    void rotationChanged(QVector3D);
    void scaleChanged(float);

private:
    void createShaderPrograms();
    void loadMesh();

    void destroyModel();
};

#endif // MAINVIEW_H
