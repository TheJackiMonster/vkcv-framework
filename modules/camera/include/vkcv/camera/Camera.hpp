#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_access.hpp>
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>

namespace vkcv::camera {

    /**
     * @brief Used to create a camera which governs the view and projection matrices.
     */
    class Camera final {
    protected:
		glm::mat4 m_view;
		glm::mat4 m_projection;

		float m_near;
		float m_far;

		glm::vec3 m_up;
        glm::vec3 m_position;
        glm::vec3 m_center;
	
		/**
		 * @brief Sets the view matrix of the camera to @p view
		 * @param[in] view The view matrix
		 */
		void setView(const glm::mat4& view);
	
		/**
		 * @brief Sets the projection matrix of the camera to @p projection
		 * @param[in] projection The projection matrix
		 */
		void setProjection(const glm::mat4& projection);

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
         * @param[in] fov The desired field of view in radians
         * @param[in] ratio The aspect ratio
         * @param[in] near Distance to near clipping plane
         * @param[in] far Distance to far clipping plane
         */
        void setPerspective(float fov, float ratio, float near, float far);

        /**
         * @brief Gets the view matrix of the camera
         * @return The view matrix of the camera
         */
        const glm::mat4& getView() const;

        /**
         * @brief Sets the view matrix of the camera according to @p position, @p center and @p up
         * @param[in] position The position of the camera
         * @param[in] center The target position the camera is looking at
         * @param[in] up The vector that defines which direction is 'up' depending on the camera's orientation
         */
        void lookAt(const glm::vec3& position, const glm::vec3& center, const glm::vec3& up);

        /**
         * @brief Gets the current projection of the camera
         * @return The current projection matrix
         */
        const glm::mat4& getProjection() const;

        /**
         * @brief Gets the model-view-projection matrix of the camera with y-axis-correction applied
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
        float getFov() const;

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
         * @brief Sets the front vector of the camera in world space to @p front
         * @param[in] front The new front vector of the camera
         */
        void setFront(const glm::vec3& front);

        /**
         * @brief Gets the current position of the camera in world space
         * @return The current position of the camera in world space
         */
        const glm::vec3& getPosition() const;

        /**
         * @brief Sets the position of the camera to @p position
         * @param[in] position The new position of the camera
         */
        void setPosition( const glm::vec3& position );

        /**
         * @brief Gets the center point.
         * @return The center point.
         */
        const glm::vec3& getCenter() const;

        /**
         * @brief Sets @p center as the new center point.
         * @param[in] center The new center point.
         */
        void setCenter(const glm::vec3& center);
        
        /**
         * @brief Gets the angles of the camera.
         * @param[out] pitch The pitch value in radians
         * @param[out] yaw The yaw value in radians
         */
		void getAngles(float& pitch, float& yaw);
  
		/**
		 * @brief Sets the angles of the camera.
		 * @param pitch The new pitch value in radians
		 * @param yaw The new yaw value in radians
		 */
		void setAngles(float pitch, float yaw);

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
        const glm::vec3& getUp() const;

        /**
         * @brief Sets @p up as the new up vector.
         * @param[in] up The new up vector.
         */
        void setUp(const glm::vec3 &up);
    };

}
