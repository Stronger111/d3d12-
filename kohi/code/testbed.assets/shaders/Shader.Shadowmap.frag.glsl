#version 450

layout(location = 0) out vec4 out_colour;

void main() {
    float alpha = texture(colour_map, in_dto.tex_coord).a;
}
