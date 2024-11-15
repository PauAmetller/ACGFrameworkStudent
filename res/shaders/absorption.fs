#version 450 core

in vec3 v_position;
in vec3 v_world_position;
in vec3 v_normal;

uniform vec3 u_camera_position;

uniform vec4 u_background_color;

uniform float u_step_length;
uniform float u_absorption_coefficient;

//Noise
uniform float u_noise_detail;
uniform float u_noise_scale;

//VDB
uniform sampler3D u_texture;

uniform int u_density_type;
#define CONSTANT 0
#define NOISE_3D 1
#define VDB 2

out vec4 FragColor;

// Noise functions
float hash1( float n )
{
    return fract( n*17.0*fract( n*0.3183099 ) );
}

float noise( vec3 x )
{
    vec3 p = floor(x);
    vec3 w = fract(x);
    
    vec3 u = w*w*w*(w*(w*6.0-15.0)+10.0);
    
    float n = p.x + 317.0*p.y + 157.0*p.z;
    
    float a = hash1(n+0.0);
    float b = hash1(n+1.0);
    float c = hash1(n+317.0);
    float d = hash1(n+318.0);
    float e = hash1(n+157.0);
    float f = hash1(n+158.0);
    float g = hash1(n+474.0);
    float h = hash1(n+475.0);

    float k0 =   a;
    float k1 =   b - a;
    float k2 =   c - a;
    float k3 =   e - a;
    float k4 =   a - b - c + d;
    float k5 =   a - c - e + g;
    float k6 =   a - b - e + f;
    float k7 = - a + b + c - d + e - f - g + h;

    return -1.0+2.0*(k0 + k1*u.x + k2*u.y + k3*u.z + k4*u.x*u.y + k5*u.y*u.z + k6*u.z*u.x + k7*u.x*u.y*u.z);
}

#define MAX_OCTAVES 16

float fractal_noise( vec3 P, float detail )
{
    float fscale = 1.0;
    float amp = 1.0;
    float sum = 0.0;
    float octaves = clamp(detail, 0.0, 16.0);
    int n = int(octaves);

    for (int i = 0; i <= MAX_OCTAVES; i++) {
        if (i > n) continue;
        float t = noise(fscale * P);
        sum += t * amp;
        amp *= 0.5;
        fscale *= 2.0;
    }

    return sum;
}

float cnoise( vec3 P, float scale, float detail )
{
    P *= scale;
    return clamp(fractal_noise(P, detail), 0.0, 1.0);
}


void initializeRay(out vec3 ray_origin, out vec3 ray_direction)
{
	ray_origin = u_camera_position;
	ray_direction = normalize(v_world_position - ray_origin);	
}

bool intersections(vec3 ray_origin, vec3 ray_direction, vec3 box_min, vec3 box_max, out float t_near, out float t_far) {
    vec3 t_min = (box_min - ray_origin) / ray_direction;
    vec3 t_max = (box_max - ray_origin) / ray_direction;
    vec3 t1 = min(t_min, t_max);
    vec3 t2 = max(t_min, t_max);
    t_near = max(max(t1.x, t1.y), t1.z);
    t_far = min(min(t2.x, t2.y), t2.z);
    return t_near <= t_far && t_far > 0.0;
}

//Homogeneous
void rayMarchingHomo(vec3 ray_origin, vec3 ray_direction, float t_near, float t_far, out vec3 radiance) {

    // Initialize parameters                       
    float optical_thickness = 0.0;

    // Compute the transmittance
    optical_thickness += u_absorption_coefficient * (t_far - t_near);

    float transmittance = exp(-optical_thickness);

    radiance = transmittance * u_background_color.xyz;

}


//Heterogenous
void rayMarching(vec3 ray_origin, vec3 ray_direction, float t_near, float t_far, out vec3 radiance) {

    // Initialize parameters
    float step_length = u_step_length;           
    float t = t_near;             
    float optical_thickness = 0.0;
    vec3 current_pos = ray_origin + t_near * ray_direction;
    float particle_density;

    // Compute the transmittance
    while (t < t_far){
        if (u_density_type == VDB) { // VDB file
            particle_density = texture(u_texture, (current_pos + vec3(1.0)) / 2.0).r; //Remap the current pos since u_texture goes from 0 to 1
        } else if (u_density_type == NOISE_3D) { // 3D Noise
            particle_density = cnoise(current_pos, u_noise_scale, u_noise_detail);
        }
        
        float absorption_coefficient = particle_density * u_absorption_coefficient;
        optical_thickness += absorption_coefficient * step_length;

        if(optical_thickness > 7){
            break;
        }
        t += step_length;
        current_pos = ray_origin + t * ray_direction;
    }
    float transmittance = exp(-optical_thickness);

    radiance = transmittance * u_background_color.xyz;

}

void main() {
    vec3 ray_origin, ray_direction;
    initializeRay(ray_origin, ray_direction);
    
    // Compute the volume intersection
    vec3 box_min = vec3(-1.0, -1.0, -1.0);  // Define your volume's min bounds
    vec3 box_max = vec3(1.0, 1.0, 1.0);     // Define your volume's max bounds
    float t_near, t_far;
    
    // If ray intersects the volume, we perform ray marching
    if (intersections(ray_origin, ray_direction, box_min, box_max, t_near, t_far)) {
        vec3 radiance;
	if(u_density_type == CONSTANT){
		rayMarchingHomo(ray_origin, ray_direction, t_near, t_far, radiance);
	} else {
		rayMarching(ray_origin, ray_direction, t_near, t_far, radiance);
	}
        
        FragColor = vec4(radiance, 1.0);
    } 
    else {
        // If no intersection, output the background color
        FragColor = u_background_color;
    }
}