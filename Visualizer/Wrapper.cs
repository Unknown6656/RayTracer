using System.Runtime.InteropServices;
using System.Text;


namespace Visualizer
{
    public enum RenderMode
    {
        Colors,
        Wireframe,
        UVCoords,
        Depths,
        SurfaceNormals,
        RayIncidenceAngle,
        RayDirection,
        Iterations,
        RenderTime,
    }

    public struct Vec3
    {
        public double X, Y, Z;

        public Vec3(double x, double y, double z) : this() => (X, Y, Z) = (x, y, z);
    }

    public struct CameraConfiguration
    {
        public Vec3 Position;
        public Vec3 LookAt;
        public double ZoomFactor;
        public double FocalLength;
    }

    //[StructLayout(LayoutKind.Sequential)]
    public struct RenderConfiguration
    {
        public ulong HorizontalResolution;
        public ulong VerticalResolution;
        public ulong SubpixelsPerPixel;
        public ulong SamplesPerSubpixel;
        public ulong MaximumIterationCount;
        public CameraConfiguration Camera;
        public RenderMode RenderMode;
    };

    public unsafe struct Scene
    {
        public const int SIZE = 24;
        public fixed byte RawBuffer[SIZE];

        public override string ToString()
        {
            StringBuilder sb = new StringBuilder().Append("[ ");

            for (int i = 0; i < SIZE; ++i)
                sb.Append($"{RawBuffer[i]:x2} ");

            return sb.Append(']').ToString();
        }
    }

    public static class RayTracer
    {
        [DllImport("RayTracer.dll", CallingConvention = CallingConvention.Cdecl)]
        public static unsafe extern void CreateScene(out Scene scene);

        [DllImport("RayTracer.dll", CallingConvention = CallingConvention.Cdecl)]
        public static unsafe extern void RenderImage(ref Scene scene, RenderConfiguration config, (double A, double R, double G, double B)* buffer, ref double progress);
    }
}
