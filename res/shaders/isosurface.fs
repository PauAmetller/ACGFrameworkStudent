#version 450 core

in vec3 v_position;
in vec3 v_world_position;
in vec3 v_normal;

uniform vec3 u_camera_position;

uniform vec4 u_color;
uniform vec4 u_background_color;

uniform vec3 u_kd;
uniform vec3 u_ks;
uniform float u_alpha;

uniform float u_step_length;

uniform float u_threshold;
uniform float u_h;

//Noise
uniform float u_noise_detail;
uniform float u_noise_scale;

//VDB
uniform sampler3D u_texture;

uniform bool u_illumination_activated;
//light
uniform float u_light_intensity;
uniform vec4 u_light_color;
uniform vec3 u_local_light_position;

uniform vec3 u_ambient_term;

//Jittering filter
uniform bool u_use_jittering;

uniform int u_density_type;
#define CONSTANT 0
#define NOISE_3D 1
#define VDB 2

out vec4 FragColor;

//Random function for the offset
float random(vec2 st) {
    return fract(sin(dot(st.xy,
                         vec2(12.9898,78.233)))*
        43758.5453123);
}

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

float getDensity(vec3 pos){
    if (u_density_type == CONSTANT){
        return 1.0;
    } else if (u_density_type == VDB) { // VDB file
        return texture(u_texture, (pos + vec3(1.0)) / 2.0).r; //Remap the current pos since u_texture goes from 0 to 1
    } else if (u_density_type == NOISE_3D) { // 3D Noise
        return cnoise(pos, u_noise_scale, u_noise_detail);
    }
}

float checkBoundsAndGetDensity(vec3 pos){

    vec3 box_min = vec3(-1.0, -1.0, -1.0);  // Define your volume's min bounds
    vec3 box_max = vec3(1.0, 1.0, 1.0);     // Define your volume's max bounds

    if (pos.x >= box_min.x && pos.x <= box_max.x &&
        pos.y >= box_min.y && pos.y <= box_max.y &&
        pos.z >= box_min.z && pos.z <= box_max.z) {
        return getDensity(pos);
    } else {
        return 0.0;
    }
}

void initializeRay(out vec3 ray_origin, out vec3 ray_direction)
{
	ray_origin = u_camera_position;
	ray_direction = normalize(v_position - ray_origin);	
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

bool CheckVisivilityWithRayMarching(vec3 rayToLight_origin, vec3 rayToLight_direction, float t_far){
    float step_length = u_step_length;
    float t = step_length;
    vec3 current_pos = rayToLight_origin + t * rayToLight_direction;

    while (t < t_far){
        
        if (getDensity(current_pos) > u_threshold){
            return false;
        }

        t += step_length;
        current_pos = rayToLight_origin + t * rayToLight_direction;
    }
    return true;
}

bool CheckVisibility(vec3 light_ray, vec3 current_pos){

    // Compute the volume intersection
    vec3 box_min = vec3(-1.0, -1.0, -1.0);  // Define your volume's min bounds
    vec3 box_max = vec3(1.0, 1.0, 1.0);     // Define your volume's max bounds
    float t_near, t_far;

	//Check intersections
	if (intersections(current_pos, light_ray, box_min, box_max, t_near, t_far)) {
        return CheckVisivilityWithRayMarching(current_pos, light_ray, t_far);
    } 
    else {
    	return true;
    }
}

const float PI = 3.14159265359;
vec3 getReflectance(vec3 normal, vec3 wo, vec3 wi) {

    vec3 wr = 2.0 * dot(wi, normal) * normal - wi;

    // Compute diffuse reflectance and specular reflectance
    vec3 diffuse = u_kd / PI; 
    vec3 specular = u_ks * (2.0 * PI / (u_alpha + 1.0)) * pow(max(dot(wo, wr), 0.0), u_alpha);

    // Combine diffuse and specular terms
    return diffuse + specular;
}

vec3 ComputeGradient(vec3 pos){
    float x = checkBoundsAndGetDensity(pos + vec3(u_h,0.0,0.0)) - checkBoundsAndGetDensity(pos - vec3(u_h,0.0,0.0));
    float y = checkBoundsAndGetDensity(pos + vec3(0.0,u_h,0.0)) - checkBoundsAndGetDensity(pos - vec3(0.0,u_h,0.0));
    float z = checkBoundsAndGetDensity(pos + vec3(0.0,0.0,u_h)) - checkBoundsAndGetDensity(pos - vec3(0.0,0.0,u_h));
    return 1/(2*u_h) * vec3(x, y, z);
}

vec3 ComputeRadianceWithIllumination(vec3 pos, vec3 ray_direction){
    vec3 normal = normalize(-ComputeGradient(pos));
    vec3 light_ray = normalize(u_local_light_position - pos);
    vec3 reflectance = getReflectance(normal, -ray_direction, light_ray);
    float visibility = CheckVisibility(light_ray, pos) ? 1.0 : 0.0;

    return visibility *(u_light_color.rgb * u_light_intensity) * dot(light_ray, normal) * reflectance + u_ambient_term;
}


//Heterogenous
void rayMarching(vec3 ray_origin, vec3 ray_direction, float t_near, float t_far, out vec4 radiance) {

    // Initialize parameters
    float step_length = u_step_length;           
    float t = t_near; 
    if (u_use_jittering){
        float offset = random(gl_FragCoord.xy) * step_length;
        if((t + offset) < t_far){
            t += offset;
        }
    }               
    vec3 current_pos = ray_origin + t * ray_direction;
    float particle_density;
    radiance = u_background_color;

    // Compute the transmittance
    while (t < t_far){
        particle_density = getDensity(current_pos);
        
        if (particle_density > u_threshold){
            if(u_illumination_activated){
                radiance = vec4(ComputeRadianceWithIllumination(current_pos, ray_direction), 1.0);
            } else {
                radiance = u_color;
            }
            break;
        }

        t += step_length;
        current_pos = ray_origin + t * ray_direction;
    }
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
        vec4 radiance;
		rayMarching(ray_origin, ray_direction, t_near, t_far, radiance);
        
        FragColor = radiance;
    } 
    else {
        // If no intersection, output the background color
        FragColor = u_background_color;
    }
}