#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_access.hpp>

namespace vkcv {

    /**
     * @brief Used to create a camera whose position can be changed.
     */
    class Camera {
    protected:
		glm::mat4 m_view;
		glm::mat4 m_projection;

		int m_width;
		int m_height;

		float m_near;
		float m_far;
		float m_fov;
		float m_ratio;

        glm::vec3 m_up;
        glm::vec3 m_position;
        float m_cameraSpeed;
        float m_pitch;
        float m_yaw;

        int m_fov_nsteps;
        float m_fov_min;
        float m_fov_max;

        bool m_forward;
        bool m_backward;
        bool m_left;
        bool m_right;

    public:

        /**
         * @brief The default constructor of the camera
         */
        Camera();

        /**
         * @brief The destructor of the camera (default behavior)
         */
        virtual ~Camera();
        
        /**
         * @brief Sets the perspective object according to @p fov, @p ratio, @p near and @p far. This leads to changes in the projection matrix of the camera
         * @param fov The desired field of view in radians
         * @param ratio The aspect ratio
         * @param near Distance to near clipping plane
         * @param far Distance to far clipping plane
         */
        void setPerspective(float fov, float ratio, float near, float far);

        /**
         * @brief Gets the view matrix of the camera
         * @return The view matrix of the camera
         */
        const glm::mat4 getView() const;
        
        /**
         * @brief Gets the view object containing the @p x, @p y, @p z axis in camera space, and the camera position @p pos in world space
         * @param x The horizontal axis in camera space
         * @param y The vertical axis in camera space
         * @param z The depth axis in camera space
         * @param pos The position of the camera in world space
         */
        void getView(glm::vec3 &x, glm::vec3 &y, glm::vec3 &z, glm::vec3 &pos);

        /**
         * @brief Updates the view matrix of the camera with respect to @p deltatime
         * @param deltatime The time past between frames
         * @return
         */
        glm::mat4 updateView(double deltatime);

        /**
         * @brief Sets the view matrix of the camera according to @p position, @p center and @p up
         * @param[out] position The position of the camera
         * @param[out] center The target position the camera is looking at
         * @param[out] up The vector that defines which direction is 'up' depending on the camera's orientation
         */
        void lookAt(glm::vec3 position, glm::vec3 center, glm::vec3 up);

        /**
         * @brief Gets the current projection of the camera
         * @return The current projection matrix
         */
        const glm::mat4& getProjection() const;

        /**
         * @brief Sets the projection matrix of the camera to @p projection
         * @param[in] projection The projection matrix
         */
        void setProjection(const glm::mat4 projection);

        /**
         * @brief Gets the near and far bounds of the view frustum of the camera.
         * @param[out] near The near bound of the view frustum
         * @param[out] far The far bound of the view frustum
         */
        void getNearFar(float &near, float &far) const;

        /**
         * @brief Sets the up vector of the camera to @p up
         * @param[in] up The new up vector of the camera
         */
        void setUp(const glm::vec3 &up);

        /**
         * @brief Gets the current field of view of the camera in radians
         * @return[in] The current field of view in radians
         */
        float getFov() const;

        /**
         * @brief Sets the field of view of the camera to @p fov in radians
         * @param[in] fov The new field of view in radians
         */
        void setFov(float fov);
        
        /**
         * @brief Changes the field of view of the camera with an @p offset in degrees
         * @param[in] offset in degrees
         */
        void changeFov(double offset);

        /**
         * @brief Updates the aspect ratio of the camera with @p ratio and, thus, changes the projection matrix
         * @param[in] ratio The new aspect ratio of the camera
         */
        void updateRatio(float ratio);

        /**
         * @brief Gets the current aspect ratio of the camera
         * @return The current aspect ratio of the camera
         */
        float getRatio() const;

        /**
         * @brief Sets @p near and @p far as new values for the view frustum of the camera. This leads to changes in the projection matrix according to these two values.
         * @param[in] near The new near bound of the view frustum
         * @param[in] far The new far bound of the view frustum
         */
        void setNearFar(float near, float far);

        /**
         * @brief Gets the current front vector of the camera in world space
         * @return The current front vector of the camera
         */
        glm::vec3 getFront() const;

        /**
         * @brief Gets the current position of the camera in world space
         * @return The current position of the camera in world space
         */
        glm::vec3 getPosition() const;

        /**
         * @brief Sets the position of the camera to @p position
         * @param[in] position The new position of the camera
         */
        void setPosition( glm::vec3 position );

        /**
         * @brief Gets the pitch value of the camera in degrees
         * @return The pitch value in degrees
         */
        float getPitch() const;

        /**
         * @brief Sets the pitch value of the camera to @p pitch in degrees
         * @param[in] pitch The new pitch value in degrees
         */
        void setPitch(float pitch);

        /**
         * @brief Gets the yaw value of the camera in degrees
         * @return The yaw value in degrees
         */
        float getYaw() const;

        /**
         * @brief Sets the yaw value of the camera to @p yaw
         * @param[in] yaw The new yaw value in degrees
         */
        void setYaw(float yaw);

        /**
         * @brief Pans the view of the camera according to the pitch and yaw values and additional offsets @p xOffset and @p yOffset (e.g. taken from mouse movement)
         * @param[in] xOffset The offset added to the yaw value
         * @param[in] yOffset The offset added to the pitch value
         */
        void panView( double xOffset, double yOffset );

        /**
         * @brief Updates the position of the camera with respect to @p deltaTime
         * @param[in] deltaTime The time that has passed since last update
         */
        void updatePosition(double deltaTime);

        /**
         * @brief Indicates forward movement of the camera depending on the performed @p action
         * @param[in] action The performed action
         */
        void moveForward(int action);

        /**
         * @brief Indicates backward movement of the camera depending on the performed @p action
         * @param[in] action The performed action
         */
        void moveBackward(int action);

        /**
         * @brief Indicates left movement of the camera depending on the performed @p action
         * @param[in] action The performed action
         */
        void moveLeft(int action);

        /**
         * @brief Indicates right movement of the camera depending on the performed @p action
         * @param[in] action The performed action
         */
        void moveRight(int action);

    };

}
