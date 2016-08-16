#version 330 core

in vec4 position;

out vec4 color;

void main()
{
	//color = texture(tex, frag_tex_coord);
	color = vec4(1.0, 1.0, 0.0, 1.0);
}