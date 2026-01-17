import math

# Stress parameters: 32x24 segments = ~1,500 triangles
COLOR = 30
STEPS_U = 32 # Radial segments (the "big circle")
STEPS_V = 24 # Tubular segments (the "small circle")
R = 100.0    # Major Radius
r = 40.0     # Minor Radius (tube thickness)

def generate_torus():
    for i in range(STEPS_U):
        # Each 'i' is a new strip
        for j in range(STEPS_V + 1):
            # We generate two vertices per step to build the quad strip
            # Vertex A (Current Ring) and Vertex B (Next Ring)
            for ring in [i, i + 1]:
                u = (ring % STEPS_U) * 2.0 * math.pi / STEPS_U
                v = (j % STEPS_V) * 2.0 * math.pi / STEPS_V
                
                # Torus Parametric Equations
                x = (R + r * math.cos(v)) * math.cos(u)
                y = (R + r * math.cos(v)) * math.sin(u)
                z = r * math.sin(v)
                
                # Scale to your i16 space (e.g., * 100) and emit
                emit_vertex(int(x * 10), int(y * 10), int(z * 10), COLOR, 0)
        
        # VERY IMPORTANT: End the strip after each full tube rotation
        emit_eos()

def emit_vertex(x, y, z, color, flags):
    print(f"{x}, {y}, {z}, {color} << 8 | {flags},")

def emit_eos():
    emit_vertex(0 ,0, 0, 0, 4 | 1)

if __name__ == "__main__":
    generate_torus()