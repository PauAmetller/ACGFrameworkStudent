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

//Emissive
uniform vec4 u_emitted_color;
uniform int u_emitted_intensity;

//Scattering
uniform float u_scattering_coefficient;
uniform bool u_use_phase_function;
uniform float u_g;

//VDB
uniform sampler3D u_texture;

//light
uniform float u_light_intensity;
uniform vec4 u_light_color;
uniform vec3 u_local_light_position;

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

bool intersections(vec3 ray_origin, vec3 ray_direction, vec3 box_min, vec3 box_max, out float t_near, out float t_far) {
    vec3 t_min = (box_min - ray_origin) / ray_direction;
    vec3 t_max = (box_max - ray_origin) / ray_direction;
    vec3 t1 = min(t_min, t_max);
    vec3 t2 = max(t_min, t_max);
    t_near = max(max(t1.x, t1.y), t1.z);
    t_far = min(min(t2.x, t2.y), t2.z);
    return t_near <= t_far && t_far > 0.0;
}

void rayMarchingToLight(vec3 ray_origin, vec3 ray_direction, float t_far, out vec3 in_scattered_color){ 
	// Initialize parameters
    float step_length = u_step_length;           
	float t = 0.0;             
	float optical_thickness = 0.0;
	vec3 current_pos = ray_origin;
	float particle_density;
    float absorption_coefficient;

    // Compute the transmittance
    while (t < t_far){
        if (u_density_type == CONSTANT){
            particle_density = 1.0;
        } else if (u_density_type == VDB) { // VDB file
            particle_density = texture(u_texture, (current_pos + vec3(1.0)) / 2.0).r;  //Remap the current pos since u_texture goes from 0 to 1
        } else if (u_density_type == NOISE_3D) { // 3D Noise
            particle_density = cnoise(current_pos, u_noise_scale, u_noise_detail);
        }

        absorption_coefficient = particle_density * u_absorption_coefficient;

        optical_thickness += absorption_coefficient * step_length;

        if(optical_thickness > 7){
            break;
        }

        t += step_length;
        current_pos = ray_origin + t * ray_direction;
    }

    float transmittance = exp(-optical_thickness);

    in_scattered_color = transmittance * u_light_color.xyz;
}

void CalculateInScattering(vec3 current_pos, out vec3 in_scattered_color)
{
	// Initialize rayToLight
	vec3 rayToLight_origin = current_pos;
	vec3 rayToLight_direction = normalize(u_local_light_position - rayToLight_origin);

	// Compute the volume intersection
    vec3 box_min = vec3(-1.0, -1.0, -1.0);  // Define your volume's min bounds
    vec3 box_max = vec3(1.0, 1.0, 1.0);     // Define your volume's max bounds
    float t_near, t_far;

	//Check intersections
	if (intersections(rayToLight_origin, rayToLight_direction, box_min, box_max, t_near, t_far)) {
        rayMarchingToLight(rayToLight_origin, rayToLight_direction, t_far, in_scattered_color);
    } 
    else {
    	in_scattered_color = vec3(0.0);
    }
}

const float PI = 3.14159265359;
float phase_function(vec3 camera_ray_direction, vec3 light_ray_direction){
    float g_square = pow(u_g, 2.0);
    float cosine_between_rays = dot(camera_ray_direction, light_ray_direction) / (length(camera_ray_direction) * length(light_ray_direction));
    return (1.0 / 4.0 * PI) * ((1 - g_square) / pow(1 + g_square - 2.0 * u_g * cosine_between_rays, 1.5));
}

void initializeRay(out vec3 ray_origin, out vec3 ray_direction)
{
	ray_origin = u_camera_position;
	ray_direction = normalize(v_world_position - ray_origin);	
}

//With step_length
void rayMarching(vec3 ray_origin, vec3 ray_direction, float t_near, float t_far, out vec3 radiance) {

    // Initialize parameters
    float step_length = u_step_length;           
    float t = t_near;             
    float optical_thickness = 0.0;
    vec3 current_pos = ray_origin + t_far * ray_direction;
    float emissive_scatter_transmittance = 0.0;
    vec3 accumulatedRadiance = vec3(0.0);
    float particle_density;
    float absorption_coefficient;
    vec3 emissive_part;
    vec3 scattering_part;
    vec3 in_scattered_color;
    float transmittance;
    vec3 light_ray;
    float phase;

    // Compute the transmittance
    while (t < t_far){
        if (u_density_type == CONSTANT){
            particle_density = 1.0;
        } else if (u_density_type == VDB) { // VDB file
            particle_density = texture(u_texture, (current_pos + vec3(1.0)) / 2.0).r; //Remap the current pos since u_texture goes from 0 to 1
        } else if (u_density_type == NOISE_3D) { // 3D Noise
            particle_density = cnoise(current_pos, u_noise_scale, u_noise_detail);
        }

        absorption_coefficient = particle_density * u_absorption_coefficient;

        optical_thickness += absorption_coefficient * step_length;

        transmittance = exp(-optical_thickness);

        emissive_part = absorption_coefficient * u_emitted_color.xyz * u_emitted_intensity;

        CalculateInScattering(current_pos, in_scattered_color);

        light_ray = normalize(u_local_light_position - current_pos);

        if(u_use_phase_function){
            phase = phase_function(ray_direction, light_ray);
        } else {
            phase = 1.0;
        }

        scattering_part = particle_density * u_scattering_coefficient * in_scattered_color * u_light_intensity * phase;
        
        accumulatedRadiance += transmittance * (emissive_part + scattering_part) * step_length;

        if(optical_thickness > 7){
            break;
        }

        t += step_length;
        current_pos = ray_origin + t * ray_direction;
    }

    radiance = accumulatedRadiance + transmittance * u_background_color.xyz;

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
        rayMarching(ray_origin, ray_direction, t_near, t_far, radiance);
        
        FragColor = vec4(radiance, 1.0);
    } 
    else {
        // If no intersection, output the background color
        FragColor = u_background_color;
    }
}