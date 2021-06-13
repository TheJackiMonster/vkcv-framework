#pragma once

#define GLM_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_LEFT_HANDED
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_access.hpp>

namespace vkcv {

    /**
     * @brief Used to create a camera which governs the view and projection matrices.
     */
    class Camera final {
    protected:
		glm::mat4 m_view;
		glm::mat4 m_projection;

		float m_near;
		float m_far;
		float m_fov;
		float m_ratio;

		glm::vec3 m_up;
        glm::vec3 m_position;
        glm::vec3 m_center;

        float m_pitch;
        float m_yaw;

    public:

        /**
         * @brief The default constructor of the camera
         */
        Camera();

        /**
         * @brief The destructor of the camera (default behavior)
         */
        ~Camera();
        
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
        glm::mat4& getView();

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
        glm::mat4& getProjection();

        /**
         * @brief Sets the projection matrix of the camera to @p projection
         * @param[in] projection The projection matrix
         */
        void setProjection(const glm::mat4 projection);

        /**
         * @brief Gets the model-view-projection matrix of the camera
         * @return The model-view-projection matrix
         */
        glm::mat4 getMVP() const;

        /**
         * @brief Gets the near and far bounds of the view frustum of the camera.
         * @param[out] near The near bound of the view frustum
         * @param[out] far The far bound of the view frustum
         */
        void getNearFar(float &near, float &far) const;

        /**
         * @brief Gets the current field of view of the camera in radians
         * @return[in] The current field of view in radians
         */
        const float getFov() const;

        /**
         * @brief Sets the field of view of the camera to @p fov in radians
         * @param[in] fov The new field of view in radians
         */
        void setFov(float fov);

        /**
         * @brief Gets the current aspect ratio of the camera
         * @return The current aspect ratio of the camera
         */
        float getRatio() const;

        /**
         * @brief Updates the aspect ratio of the camera with @p ratio and, thus, changes the projection matrix
         * @param[in] ratio The new aspect ratio of the camera
         */
        void setRatio(float ratio);

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
         * @brief Gets the center point.
         * @return The center point.
         */
        glm::vec3 getCenter() const;

        /**
         * @brief Sets @p center as the new center point.
         * @param center The new center point.
         */
        void setCenter(glm::vec3 center);

        /**
         * @brief Gets the pitch value of the camera in degrees.
         * @return The pitch value in degrees.
         */
        float getPitch() const;

        /**
         * @brief Sets the pitch value of the camera to @p pitch in degrees.
         * @param[in] pitch The new pitch value in degrees.
         */
        void setPitch(float pitch);

        /**
         * @brief Gets the yaw value of the camera in degrees.
         * @return The yaw value in degrees.
         */
        float getYaw() const;

        /**
         * @brief Sets the yaw value of the camera to @p yaw.
         * @param[in] yaw The new yaw value in degrees.
         */
        void setYaw(float yaw);

        /**
         * @brief Gets the up vector.
         * @return The up vector.
         */
        glm::vec3 getUp() const;

        /**
         * @brief Sets @p up as the new up vector.
         * @param up The new up vector.
         */
        void setUp(const glm::vec3 &up);
    };

}
