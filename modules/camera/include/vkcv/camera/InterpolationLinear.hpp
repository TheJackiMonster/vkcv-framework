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
        
        void addPosition(const glm::vec3& pos);
        
        void setCamera(Camera camera);

        void resetTimer();
        
        void updateCamera();
    };

}