# Fire works
An example project to show a particle simulation via compute shader to visualize virtual firework

![Screenshot](../../screenshots/fire_works.png)

## Details

The project uses many different compute shaders to calculate different steps of the particles
lifecycle. Particles can dynamically be spread to different events for explosions on the GPU. The 
particles will then be rendered as volumetric smoke and geometric smoke using geometry shader 
invocations.
