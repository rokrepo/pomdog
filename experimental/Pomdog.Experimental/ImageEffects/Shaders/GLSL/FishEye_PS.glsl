#version 330

in QuadVertexShaderOutput {
	vec2 TextureCoord;
} In;

uniform Constants {
	vec2 RenderTargetPixelSize;
};

uniform sampler2D Texture;

out vec4 FragColor;

const float PI = 3.1415926535;

vec2 BasicFishEye(vec2 textureCoord)
{
	const float aperture = 178.0;
	float apertureHalf = 0.5 * aperture * (PI/ 180.0);
	float maxFactror = sin(apertureHalf);
	
	vec2 xy = 2.0 * textureCoord - 1.0;
	float d = length(xy);
	
	vec2 uv = textureCoord;
	
	if (d < (2.0 - maxFactror))
	{
		d = length(xy * maxFactror);
		float z = sqrt(1.0 - d * d);
		float r = atan(d, z) / PI;
		float phi = atan(xy.y, xy.x);
		
		uv.x = r * cos(phi) + 0.5;
		uv.y = r * sin(phi) + 0.5;
	}
	
	return uv;
}

///@param strength  range: (-0.5, 0.5)
vec2 FishEye(vec2 textureCoord, vec2 resolution, float strength)
{
	//vec2 p = gl_FragCoord.xy / resolution.x;
	vec2 p = vec2(textureCoord.x,  textureCoord.y * resolution.y/resolution.x);
	
	float prop = resolution.x / resolution.y;
	vec2 m = vec2(0.5, 0.5 / prop);
	
	vec2 d = p - m;
	
	float r = sqrt(dot(d, d));
	
	float power = (PI / sqrt(dot(m, m))) * strength;
	
	float bind = m.y;
	if (power > 0.0) {
		bind = sqrt(dot(m, m));
	}
	else {
		if (prop < 1.0) {
			bind = m.x;
		} else {
			bind = m.y;
		}
	}
	
	vec2 uv;
	if (power > 0.0) {
		// fisheye
		uv = m + normalize(d) * tan(r * power) * bind / tan(bind * power);
	} else if (power < 0.0) {
		// anti-fisheye
		uv = m + normalize(d) * atan(r * -power * 10.0) * bind / atan(-power * bind * 10.0);
	} else {
		// no effect
		uv = p;
	}

	uv.y = uv.y * prop;
	return uv;
}

void main()
{
	const float strength = -0.04;
	vec2 uv = FishEye(In.TextureCoord.xy, RenderTargetPixelSize.xy, strength);

	vec4 color = texture(Texture, uv);
	
	FragColor = color;
}
