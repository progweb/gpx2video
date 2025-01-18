precision highp float;

varying vec2 vertexTexCoord;

uniform sampler2D inputTexture1;
//uniform sampler2D inputTexture2;

void main() {
	gl_FragColor = texture2D(inputTexture1, vertexTexCoord);

//	    vec3 c;
//    vec4 Ca = texture2D(inputTexture2, vertexTexCoord);
//    vec4 Cb = texture2D(inputTexture1, vertexTexCoord);
//    c = Ca.rgb * Ca.a + Cb.rgb * Cb.a * (1.0 - Ca.a);  // blending equation
//    gl_FragColor= vec4(c, 1.0);
}

