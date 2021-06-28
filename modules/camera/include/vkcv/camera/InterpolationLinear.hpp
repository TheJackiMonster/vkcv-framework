#include <glm/glm.hpp>
#include <GLFW/glfw3.h>
#include <vkcv/camera/Camera.hpp>
#include <vector>

namespace vkcv::camera {

    class InterpolationLinear{

    private:
        
        Camera& m_camera;
        double m_timeSinceStart; 
        std::vector<glm::vec3> m_positions;


    public:

        InterpolationLinear(Camera &camera);
        
        /**
         * @brief Add position vector to be approached. The positions are approached in the order in which they where added.
         * 
         * @param pos 
         */
        void addPosition(const glm::vec3& pos);
        
        /**
         * @brief Set the Camera object
         * 
         * @param camera 
         */
        void setCamera(Camera camera);

        /**
         * @brief Resets timer
         * 
         */
        void resetTimer();
        
        /**
         * @brief Updates the camera with the interpolated position values
         * 
         */
        void updateCamera();
    };

}