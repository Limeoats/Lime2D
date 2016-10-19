uniform sampler2D texture;
 uniform vec3 color;
 uniform float intensity;

 void main() {
     vec4 pixel = texture2D(texture, gl_TexCoord[0].xy);
     vec3 newColor = color.rgb * intensity;
     gl_FragColor = vec4(pixel.rgb * newColor.rgb, pixel.a);
 }