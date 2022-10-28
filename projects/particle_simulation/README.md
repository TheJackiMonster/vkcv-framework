# Particle simulation
An example project to show thousands of particles can be simulated with the VkCV framework

![Screenshot space](../../screenshots/particle_simulation_space.png)
![Screenshot water](../../screenshots/particle_simulation_water.png)
![Screenshot gravity](../../screenshots/particle_simulation_gravity.png)

## Details

Similar to the projects to show rendering a single triangle or a simple mesh was possible. This
project shows that many particles can be simulated using compute shaders and rendered using the
usual graphics pipeline, all with the VkCV framework.

The behavior of the particles can easily be adjusted loading a different compute shader. That is
the reason the application can be started with three different pipelines using one of those 
parameters:

 - `--space` to simulate particles flying around in a wild way through space
 - `--water` to simulate particles dropping down like a waterfall to the ground
 - `--gravity` to simulate particles being drawn by points of gravity in space flying around those points
