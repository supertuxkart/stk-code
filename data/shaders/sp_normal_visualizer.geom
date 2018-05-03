uniform int enable_normals;
uniform int enable_tangents;
uniform int enable_bitangents;
uniform int enable_wireframe;
uniform int enable_triangle_normals;

layout(triangles) in;
layout(line_strip, max_vertices = 24) out;

in vec3 o_normal[];
in vec3 o_tangent[];
in vec3 o_bitangent[];

flat out vec4 o_color;

void main()
{
    if (enable_normals == 0 && enable_tangents == 0 &&
        enable_bitangents == 0 && enable_wireframe == 0 &&
        enable_triangle_normals == 0)
    {
        return;
    }

    // colors for different type of new lines
    vec4 edge_color = vec4(0.2, 0.1, 0.1, 1.0);
    vec4 face_normal_color = vec4(0.5, 0.7, 0.2, 1.0);
    vec4 normal_color = vec4(0.0, 1.0, 0.2, 1.0);
    vec4 tangent_color = vec4(1.0, 0.3, 0.2, 1.0);
    vec4 bitangent_color = vec4(0.0, 0.1, 1.0, 1.0);

    // form two vectors from triangle
    vec3 V0 = gl_in[0].gl_Position.xyz - gl_in[1].gl_Position.xyz;
    vec3 V1 = gl_in[2].gl_Position.xyz - gl_in[1].gl_Position.xyz;
    // calculate normal as perpendicular to two vectors of the triangle
    vec3 V0_V1_crossed = cross(V1, V0);
    float normal_scale = min(length(V0_V1_crossed) * 10.0, 0.25);
    vec3 N = normalize(V0_V1_crossed);

    // normals of each vertex of the triangle
    vec3 nor[3];
    nor[0] = o_normal[0].xyz;
    nor[1] = o_normal[1].xyz;
    nor[2] = o_normal[2].xyz;

    // positions of each vertex of the triangle
    // shifted a bit along normal
    // so there won't be Z fighting when rendered over the mesh
    vec4 pos[3];
    pos[0] = u_projection_view_matrix *
        vec4(gl_in[0].gl_Position.xyz + nor[0] * 0.01, 1.0);
    pos[1] = u_projection_view_matrix *
        vec4(gl_in[1].gl_Position.xyz + nor[1] * 0.01, 1.0);
    pos[2] = u_projection_view_matrix *
        vec4(gl_in[2].gl_Position.xyz + nor[2] * 0.01, 1.0);

    // output normals, tangents and bitangents for each vertex of the triangle
    for (int i=0; i < gl_in.length(); i++)
    {
        // get position of the vertex
        vec3 P = gl_in[i].gl_Position.xyz;

        if (enable_normals > 0)
        {
            // create normal for vertex
            o_color = normal_color;
            gl_Position = pos[i];
            EmitVertex();
            gl_Position = u_projection_view_matrix * vec4(P + o_normal[i].xyz
                * normal_scale, 1.0);
            EmitVertex();
            EndPrimitive();
        }

        if (enable_tangents > 0)
        {
            // create tangent for vertex
            o_color = tangent_color;
            gl_Position = pos[i];
            EmitVertex();
            gl_Position = u_projection_view_matrix *
                vec4(P + o_tangent[i].xyz * normal_scale, 1.0);
            EmitVertex();
            EndPrimitive();
        }

        if (enable_bitangents > 0)
        {
            // create bitangent for vertex
            o_color = bitangent_color;
            gl_Position = pos[i];
            EmitVertex();
            gl_Position = u_projection_view_matrix * vec4(P +
                o_bitangent[i].xyz * normal_scale, 1.0);
            EmitVertex();
            EndPrimitive();
        }
    }

    if (enable_wireframe > 0)
    {
        // create edges for triangle
        o_color = edge_color;
        gl_Position = pos[0];
        EmitVertex();
        gl_Position = pos[1];
        EmitVertex();
        gl_Position = pos[2];
        EmitVertex();
        gl_Position = pos[0];
        EmitVertex();
        // end line strip after four added vertices, so we will get three lines
        EndPrimitive();
    }

    if (enable_triangle_normals > 0)
    {
        // create normal for triangle
        o_color = face_normal_color;

        // position as arithmetic average
        vec3 P = (gl_in[0].gl_Position.xyz + gl_in[1].gl_Position.xyz
            + gl_in[2].gl_Position.xyz) / 3.0;
        gl_Position = u_projection_view_matrix * vec4(P, 1.0);
        EmitVertex();
        gl_Position = u_projection_view_matrix * vec4(P + N * normal_scale, 1.0);
        EmitVertex();
        EndPrimitive();
    }
}
