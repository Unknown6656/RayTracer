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
        public float X, Y, Z;

        public Vec3(float x, float y, float z) : this() => (X, Y, Z) = (x, y, z);
    }

    public struct CameraConfiguration
    {
        public Vec3 Position;
        public Vec3 LookAt;
        public float ZoomFactor;
        public float FocalLength;
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

    public static class RayTracer
    {
        [DllImport("RayTracer.dll", CallingConvention = CallingConvention.Cdecl)]
        public static unsafe extern void CreateScene(void** scene);

        [DllImport("RayTracer.dll", CallingConvention = CallingConvention.Cdecl)]
        public static unsafe extern void DeleteScene(void** scene);

        [DllImport("RayTracer.dll", CallingConvention = CallingConvention.Cdecl)]
        public static unsafe extern void RenderImage(void* scene, RenderConfiguration config, (float A, float R, float G, float B)* buffer, ref float progress);
    }
}
