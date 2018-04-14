#include "mainview.h"
#include "model.h"

#include <cassert>
#include <cmath>
#include <math.h>
#include <QDateTime>

#include <iostream>
using namespace std;

void printVector(QVector3D vector)
{
    cout << "(" << vector.x() << ", " << vector.y() << ", " << vector.z() << ")" << endl;
}

void printMatrix(QMatrix4x4 matrix, std::string title = "")
{
    cout << "---- " << title << " ----" << endl;
    for(size_t y = 0; y < 4; ++y)
    {
        cout << "[ ";
        for(size_t x = 0; x < 4; ++x)
        {
            cout << matrix(y, x) << " ";
        }
        cout << "]" << endl;
    }
    cout << "----------------" << endl;
}

/**
 * @brief MainView::MainView
 *
 * Constructor of MainView
 *
 * @param parent
 */
MainView::MainView(QWidget *parent) : QOpenGLWidget(parent),
    shaderActiveProgram(nullptr),
    planetIllumination{0.1, 0.2, 1.0, 0.2, 5.0},
    sunIllumination{1.0, 0.2, 0.8, 0.0, 50.0},
    horseIllumination{0.2, 0.2, 1.0, 0.6, 50.0},
    universeIllumination{0.8, 0.1, 0.0, 0.0, 50.0},
    universeCenter(0.0, 0.0, -5.0),
    light{QVector3D(0.0, 0.0, -5.0), QVector3D(1.0, 1.0, 0.8)},
    m_state{QVector3D(0.0, 0.0, 0.0), 100.0},
    m_drag{0, 0, 0, QVector3D(0, 0, 0)},
    inRotationUpdate(false),
    inScaleUpdate(false)
{
    qDebug() << "MainView constructor";

    connect(&timer, SIGNAL(timeout()), this, SLOT(update()));
}

/**
 * @brief MainView::~MainView
 *
 * Destructor of MainView
 * This is the last function called, before exit of the program
 * Use this to clean up your variables, buffers etc.
 *
 */
MainView::~MainView()
{
    makeCurrent();
    debugLogger->stopLogging();
    delete debugLogger;

    qDebug() << "MainView destructor";
    for(std::shared_ptr<Mesh>& mesh : objects)
    {
        destroyMesh(mesh);
    }
    doneCurrent();
}

// --- OpenGL initialization

/**
 * @brief MainView::initializeGL
 *
 * Called upon OpenGL initialization
 * Attaches a debugger and calls other init functions
 */
void MainView::initializeGL()
{
    qDebug() << ":: Initializing OpenGL";
    initializeOpenGLFunctions();

    debugLogger = new QOpenGLDebugLogger();
    connect( debugLogger, SIGNAL( messageLogged( QOpenGLDebugMessage ) ),
             this, SLOT( onMessageLogged( QOpenGLDebugMessage ) ), Qt::DirectConnection );

    if ( debugLogger->initialize() )
    {
        qDebug() << ":: Logging initialized";
        debugLogger->startLogging( QOpenGLDebugLogger::SynchronousLogging );
        debugLogger->enableMessages();
    }

    QString glVersion;
    glVersion = reinterpret_cast<const char*>(glGetString(GL_VERSION));
    qDebug() << ":: Using OpenGL" << qPrintable(glVersion);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    // Horse tail hair is made by texture alpha channel, so we need blending
    glEnable(GL_BLEND);
    glDepthFunc(GL_LEQUAL);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glClearColor(0.0, 1.0, 0.0, 1.0);

    createShaderPrograms();
    shaderActiveProgram = &shaderPhongProgram;
    {
        // TODO: All this could be moved to a loader class
        // TODO: However, we won't do it, because of time constraints.
        std::shared_ptr<Mesh> sun = loadMesh(":/models/sphere.obj", ":/textures/sun.png", sunIllumination);
        sun->transform.base.translation = QVector3D(0.0, 0.0, -5.0);
        sun->transform.base.scale = 1.0;
        sun->transform.rotation.state.deltaValue = QVector3D(0.0, 15.0 / 60.0, 0.0);
        sun->name = "Sun";
        objects.push_back(sun);
        std::shared_ptr<Mesh> universe = loadMesh(":/models/inner-sphere.obj", ":/textures/milky-way.png", universeIllumination);
        universe->name = "Universe";
        universe->transform.base.translation = QVector3D(0.0, 0.0, -5.0);
        universe->transform.base.scale = 80.0;
        objects.push_back(universe);
        std::shared_ptr<Mesh> earth = loadMesh(":/models/sphere.obj", ":/textures/earth.png", planetIllumination);
        earth->transform.base.translation = QVector3D(5.0, 0.0, -5.0);
        earth->transform.base.scale = 0.3;
        earth->transform.rotation.state.deltaValue = QVector3D(0.0, 90.0 / 60, 0.0);
        earth->transform.orbit.pivot = QVector3D(0.0, 0.0, -5.0);
        earth->transform.orbit.state.deltaValue = QVector3D(0.0, -10.0 / 60, 0.0);
        earth->name = "Earth";
        objects.push_back(earth);
        std::shared_ptr<Mesh> mars = loadMesh(":/models/sphere.obj", ":/textures/mars.png", planetIllumination);
        mars->transform.base.translation = QVector3D(8.0, 0.0, -5.0);
        mars->transform.base.scale = 0.3;
        mars->transform.rotation.state.deltaValue = QVector3D(0.0, 60.0 / 60, 0.0);
        mars->transform.orbit.pivot = QVector3D(0.0, 0.0, -5.0);
        mars->transform.orbit.state.deltaValue = QVector3D(0.0, -15.0 / 60, 0.0);
        mars->name = "Mars";
        objects.push_back(mars);
        std::shared_ptr<Mesh> horse = loadMesh(":/models/horse.obj", ":/textures/horse.png", ":/textures/horse-normal.png", horseIllumination);
        horse->transform.base.translation = QVector3D(5.0, 1.0, -5.0);
        horse->transform.base.scale = 0.003;
        horse->transform.rotation.state.deltaValue = QVector3D(30.0 / 60, 45.0 / 60, 11.531254 / 60);
        horse->transform.orbit.pivotObject = earth;
        horse->transform.orbit.pivot = QVector3D(0.0, 0.0, -5.0);
        horse->transform.orbit.state.deltaValue = QVector3D(0.0, 0.0, 60.0 / 60);
        objects.push_back(horse);

        std::shared_ptr<Mesh> horse2 = loadMesh(":/models/horse.obj", ":/textures/horse.png", ":/textures/horse-normal.png", horseIllumination);
        horse2->transform.base.translation = QVector3D(14.0, 0.0, -5.0);
        horse2->transform.base.rotation = QVector3D(-90.0, -90.0, 0.0);
        horse2->transform.base.scale = 0.003;
        horse2->transform.orbit.pivot = QVector3D(0.0, 0.0, -5.0);
        horse2->transform.orbit.state.deltaValue = QVector3D(0.0, 0.0, 5.0 / 60.0);
        objects.push_back(horse2);

        cameraLock = sun;
    }

    // Initialize transformations
    updateViewTransform();
    updateProjectionTransform();

    timer.start(1000.0 / 60.0);
}

void MainView::createShaderPrograms()
{
    // Create shader program
    shaderNormalProgram.addShaderFromSourceFile(QOpenGLShader::Vertex,
            ":/shaders/vertshader_normal.glsl");
    shaderNormalProgram.addShaderFromSourceFile(QOpenGLShader::Fragment,
            ":/shaders/fragshader_normal.glsl");
    shaderNormalProgram.link();

    shaderGouraudProgram.addShaderFromSourceFile(QOpenGLShader::Vertex,
            ":/shaders/vertshader_gouraud.glsl");
    shaderGouraudProgram.addShaderFromSourceFile(QOpenGLShader::Fragment,
            ":/shaders/fragshader_gouraud.glsl");
    shaderGouraudProgram.link();

    shaderPhongProgram.addShaderFromSourceFile(QOpenGLShader::Vertex,
            ":/shaders/vertshader_phong.glsl");
    shaderPhongProgram.addShaderFromSourceFile(QOpenGLShader::Fragment,
            ":/shaders/fragshader_phong.glsl");
    shaderPhongProgram.link();
}

std::shared_ptr<MainView::Mesh> MainView::loadMesh(QString objFile, QString surfaceTexture, Illumination illumination)
{
    std::shared_ptr<Mesh> mesh = std::make_shared<Mesh>();
    mesh->illumination = illumination;
    Model model(objFile, Model::Flags::TANGENT);

    std::vector<float> meshData = model.getVNTTBInterleaved_indexed();
    std::vector<unsigned> meshIndices = model.getIndices();

    mesh->numIndices = meshIndices.size();

    // Generate VAO
    glGenVertexArrays(1, &mesh->vertexArrayObject);
    glBindVertexArray(mesh->vertexArrayObject);

    // Generate VBO
    glGenBuffers(1, &mesh->vertexBufferObject);
    glBindBuffer(GL_ARRAY_BUFFER, mesh->vertexBufferObject);

    // Write the data to the buffer
    glBufferData(GL_ARRAY_BUFFER, meshData.size() * sizeof(float), meshData.data(), GL_STATIC_DRAW);

    glGenBuffers(1, &mesh->indexBufferObject);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->indexBufferObject);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh->numIndices * sizeof(unsigned), meshIndices.data(), GL_STATIC_DRAW);

    // Set vertex coordinates to location 0
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), 0);
    glEnableVertexAttribArray(0);

    // Set normal coordinates to location 1
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // Set texture coordinates to location 2
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void *)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    // Set tangent coordiantes to location 3
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void *)(8 * sizeof(float)));
    glEnableVertexAttribArray(3);

    // Set bitangent coordiantes to location 4
    glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void *)(11 * sizeof(float)));
    glEnableVertexAttribArray(4);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    glGenTextures(1, &mesh->surfaceTexture);
    glGenTextures(1, &mesh->normalTexture);
    loadTexture(surfaceTexture, mesh->surfaceTexture);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, mesh->normalTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    quint8 keepNormalsColor[4] = {0x80, 0x80, 0xFF, 0xFF};
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, keepNormalsColor);
    glBindTexture(GL_TEXTURE_2D, 0);

    return mesh;
}

std::shared_ptr<MainView::Mesh> MainView::loadMesh(QString objFile, QString surfaceTexture, QString normalTexture, Illumination illumination)
{
    std::shared_ptr<Mesh> mesh = std::make_shared<Mesh>();
    mesh->illumination = illumination;
    Model model(objFile, Model::Flags::TANGENT);

    std::vector<float> meshData = model.getVNTTBInterleaved_indexed();
    std::vector<unsigned> meshIndices = model.getIndices();

    mesh->numIndices = meshIndices.size();

    // Generate VAO
    glGenVertexArrays(1, &mesh->vertexArrayObject);
    glBindVertexArray(mesh->vertexArrayObject);

    // Generate VBO
    glGenBuffers(1, &mesh->vertexBufferObject);
    glBindBuffer(GL_ARRAY_BUFFER, mesh->vertexBufferObject);

    // Write the data to the buffer
    glBufferData(GL_ARRAY_BUFFER, meshData.size() * sizeof(float), meshData.data(), GL_STATIC_DRAW);

    glGenBuffers(1, &mesh->indexBufferObject);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->indexBufferObject);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh->numIndices * sizeof(unsigned), meshIndices.data(), GL_STATIC_DRAW);

    // Set vertex coordinates to location 0
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), 0);
    glEnableVertexAttribArray(0);

    // Set normal coordinates to location 1
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // Set texture coordinates to location 2
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void *)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    // Set tangent coordiantes to location 3
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void *)(8 * sizeof(float)));
    glEnableVertexAttribArray(3);

    // Set bitangent coordiantes to location 4
    glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void *)(11 * sizeof(float)));
    glEnableVertexAttribArray(4);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    glGenTextures(1, &mesh->surfaceTexture);
    glGenTextures(1, &mesh->normalTexture);
    loadTexture(surfaceTexture, mesh->surfaceTexture);
    loadTexture(normalTexture, mesh->normalTexture);

    return mesh;
}

void MainView::destroyMesh(std::shared_ptr<Mesh>& mesh)
{
    glDeleteTextures(1, &mesh->surfaceTexture);
    glDeleteBuffers(1, &mesh->vertexBufferObject);
    glDeleteBuffers(1, &mesh->indexBufferObject);
    glDeleteVertexArrays(1, &mesh->vertexArrayObject);
}

void MainView::loadTexture(QString file, GLuint texturePtr)
{
    QImage image(file);
    std::vector<quint8> data(imageToBytes(image));
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texturePtr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, image.width(), image.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, data.data());
    glGenerateMipmap(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);
}

// --- OpenGL drawing

/**
 * @brief MainView::paintGL
 *
 * Actual function used for drawing to the screen
 *
 */
void MainView::paintGL()
{
    animate();

    // Clear the screen before rendering
    glClearColor(0.2f, 0.5f, 0.7f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    shaderActiveProgram->bind();

    for(const std::shared_ptr<Mesh>& mesh : objects)
    {
        applyUniforms(mesh);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, mesh->surfaceTexture);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, mesh->normalTexture);

        glBindVertexArray(mesh->vertexArrayObject);
        glDrawElements(GL_TRIANGLES, mesh->numIndices, GL_UNSIGNED_INT, nullptr);
    }

    glBindTexture(GL_TEXTURE_2D, 0);
    glBindVertexArray(0);

    shaderActiveProgram->release();
}

QMatrix4x4 MainView::getMeshTransform(const std::shared_ptr<const MainView::Mesh>& mesh) const
{
    QMatrix4x4 transform;

    if(mesh->transform.orbit.pivotObject)
    {
        // First we need to retrieve how the two objects' initially stay in space (e.g. their relation)
        // This is the vector from the pivot object to this object
        QVector3D orbitInitial = mesh->transform.base.translation - mesh->transform.orbit.pivotObject->transform.base.translation;

        // Now we need to know where the other object is actually is,
        // Since model space locate all objects at (0, 0, 0), we just transform that point:
        QVector3D pivot = getMeshTransform(mesh->transform.orbit.pivotObject) * QVector3D(0.0, 0.0, 0.0);

        // Now instead of moving the object into its world coordinates, we move it into the pivot object
        // Usually if the pivot was in its original location, we would move this object to the world space then into the pivot, rotate and return it.
        // However, this does not work well, because the pivot can be moving (and in this case it is).
        // So we directly translate this object into the current pivot position, then rotate and use its original orbit vector from above to move it back.
        transform.translate(pivot);

        // Make an orbit around itself
        transform.rotate(QQuaternion::fromEulerAngles(mesh->transform.orbit.state.currentValue));

        // "Return" the object into its initial position based on the orbitInitial
        // If no rotation is performed (0, 0, 0) this should match original world coordinates of the object
        transform.translate(orbitInitial);

    }
    else
    {
        QVector3D orbitInitial = mesh->transform.base.translation - mesh->transform.orbit.pivot;

        transform.translate(mesh->transform.orbit.pivot);

        // Translate the object into its world coordinates
        transform.rotate(QQuaternion::fromEulerAngles(mesh->transform.orbit.state.currentValue));

        // ... and then return it, but because of the rotation, the object is returned at different location
        // on the same distance from the orbit point as it was before the rotation.
        transform.translate(orbitInitial);
    }
    // Since the object is in the correct position, no more translation should be applied...

    // Rotate the object to its initial rotation
    transform.rotate(QQuaternion::fromEulerAngles(mesh->transform.base.rotation));

    // Rotate again for animating rotation around itself
    // TODO: Probably is better to use axis rotation here, since we can incline the earth with base rotation and apply axis rotation here.
    // Note (to self): QQuarternion::fromAxisAndAngle could be used, however axis is better fit the Z-rotation from the base transform.
    transform.rotate(QQuaternion::fromEulerAngles(mesh->transform.rotation.state.currentValue));

    // Finally we scale the model (especially important for the horse, since units there are "cm" (denoted in the model we downloaded))
    transform.scale(mesh->transform.base.scale);

    return transform;
}

void MainView::applyUniforms(const std::shared_ptr<Mesh>& mesh)
{
    shaderActiveProgram->setUniformValue("projectionTransform", projectionTransform);
    shaderActiveProgram->setUniformValue("viewTransform", viewTransform);
    QMatrix4x4 meshTransform = getMeshTransform(mesh);
    shaderActiveProgram->setUniformValue("modelTransform", meshTransform);
    shaderActiveProgram->setUniformValue("normalModelTransform", meshTransform.normalMatrix());
    shaderActiveProgram->setUniformValue("normalViewTransform", viewTransform.normalMatrix());
    if(activeMode == ShadingMode::GOURAUD || activeMode == ShadingMode::PHONG)
    {
        shaderActiveProgram->setUniformValue("light.position", light.position);
        shaderActiveProgram->setUniformValue("light.color", light.color);
        shaderActiveProgram->setUniformValue("material.emissionFactor", mesh->illumination.emissionFactor);
        shaderActiveProgram->setUniformValue("material.ambientFactor", mesh->illumination.ambientFactor);
        shaderActiveProgram->setUniformValue("material.diffuseFactor", mesh->illumination.diffuseFactor);
        shaderActiveProgram->setUniformValue("material.specularFactor", mesh->illumination.specularFactor);
        shaderActiveProgram->setUniformValue("material.specularExponent", mesh->illumination.specularExponent);
        shaderActiveProgram->setUniformValue("texImage", 0);
        shaderActiveProgram->setUniformValue("normalImage", 1);
    }
}

/**
 * @brief MainView::resizeGL
 *
 * Called upon resizing of the screen
 *
 * @param newWidth
 * @param newHeight
 */
void MainView::resizeGL(int newWidth, int newHeight)
{
    Q_UNUSED(newWidth)
    Q_UNUSED(newHeight)
    updateProjectionTransform();
}

void MainView::updateProjectionTransform()
{
    float aspect_ratio = static_cast<float>(width()) / static_cast<float>(height());
    projectionTransform.setToIdentity();
    projectionTransform.perspective(60, aspect_ratio, 0.1, 400.0);
    update();
}

void MainView::updateViewTransform()
{
    QVector3D modelFront(0, 0, -1); // Where is front (towards far point)?
    QVector3D modelUp(0, 1, 0); // Where is up?
    QVector3D modelRight(1, 0, 0); // Where is right?

    // Find the rotation itself
    QMatrix4x4 cameraRotationMatrix;
    cameraRotationMatrix.rotate(QQuaternion::fromEulerAngles(m_state.rotation));

    // Now we know where front, up and right is after rotation.
    QVector3D viewFront = cameraRotationMatrix * modelFront;
    QVector3D viewUp = cameraRotationMatrix * modelUp;
    QVector3D viewRight = cameraRotationMatrix * modelRight;

    // This is how we rotate based on gluLookAt function. We look toward specific direction.
    // This is quite enough for first person mode.
    QMatrix4x4 cameraTransform;
    cameraTransform.setToIdentity();
    cameraTransform.setRow(0, QVector3D(viewRight));
    cameraTransform.setRow(1, QVector3D(viewUp));
    cameraTransform.setRow(2, -QVector3D(viewFront));

    QVector3D lockPoint = universeCenter;
    if(cameraLock)
    {
        QMatrix4x4 objectFollowTransform = getMeshTransform(cameraLock);
        lockPoint = objectFollowTransform * QVector3D(0, 0, 0);
    }

    QVector3D cameraCenter = cameraTransform * (-lockPoint);
    QMatrix4x4 cameraTranslate;
    cameraTranslate.translate(cameraCenter);
    // 10^x passes through (0, 1), (1, 10), (2, 100)
    // 0.602 * 18.2727^x - 1 = passes through (0, 0), (1, 10), (2, 200)
    float zoom = 0.602f * pow(18.2727f, m_state.scale / 100.0f) - 1.0f;
    cameraTranslate.translate(0.0f, 0.0f, -zoom);

    // TODO: Unfortunately, the rotation is based on the world coordinates, which seems incorrect after Z-rotation
    // TODO: Z-rotation should affect the XY-rotation, but it does not.
    // TODO: Around the x axis there lies the milky way. Milky way keep its rotation when Z changed.

    viewTransform = cameraTranslate * cameraTransform;

    update();
}

// --- Public interface

void MainView::setRotation(int rotateX, int rotateY, int rotateZ)
{
    m_state.rotation = { static_cast<float>(rotateX), static_cast<float>(rotateY), static_cast<float>(rotateZ) };
    qDebug() << "View Rotation: [" << m_state.rotation.x() << ", " << m_state.rotation.y() << ", " << m_state.rotation.z() << "]";
    updateViewTransform();
}

void MainView::setScale(int newScale)
{
    m_state.scale = static_cast<float>(newScale);
    qDebug() << "Zoom: " << m_state.scale;
    updateViewTransform();
}

void MainView::setShadingMode(ShadingMode shading)
{
    qDebug() << "Changed shading to" << shading;
    switch(shading)
    {
    case ShadingMode::PHONG:
        shaderActiveProgram = &shaderPhongProgram;
        break;
    case ShadingMode::NORMAL:
        shaderActiveProgram = &shaderNormalProgram;
        break;
    case ShadingMode::GOURAUD:
        shaderActiveProgram = &shaderGouraudProgram;
        break;
    default:
        throw std::runtime_error("Invalid shader program selected.");
    }
    activeMode = shading;
}

// --- Private helpers

/**
 * @brief MainView::onMessageLogged
 *
 * OpenGL logging function, do not change
 *
 * @param Message
 */
void MainView::onMessageLogged( QOpenGLDebugMessage Message )
{
    qDebug() << " → Log:" << Message;
}

void MainView::animate()
{
    for(const std::shared_ptr<Mesh>& mesh : objects)
    {
        mesh->transform.rotation.state.currentValue += mesh->transform.rotation.state.deltaValue;
        mesh->transform.orbit.state.currentValue += mesh->transform.orbit.state.deltaValue;
        updateViewTransform();
    }
}

void MainView::setCameraLock(size_t index)
{
    cameraLock = objects.at(index);
    qDebug() << "Locking on " << cameraLock->name.c_str();
    updateViewTransform();
}
