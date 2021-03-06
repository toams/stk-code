#version 130
uniform sampler2D tex;
uniform sampler2D dtex;

uniform int viz;

in vec2 uv;

void main()
{
	vec2 tc = uv;

	vec4 shiftval = texture2D(dtex, tc) / vec4(50.0);
	vec2 shift;
	shift.x = -shiftval.x + shiftval.y;
	shift.y = -shiftval.z + shiftval.w;

	tc += shift;

	vec4 newcol = texture2D(tex, tc);

	if (viz < 1)
	{
		gl_FragColor = newcol;
	} else
	{
		gl_FragColor = shiftval * vec4(50.0);
	}
}
