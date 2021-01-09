using System.Runtime.InteropServices;


namespace Visualizer
{
    public enum RenderMode
    {
        Colors,
        Depths,
        SurfaceNormals,
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

    public static class RayTracer
    {
        [DllImport("RayTracer.dll", CallingConvention = CallingConvention.Cdecl)]
        public static unsafe extern void RenderImage(RenderConfiguration config, (double A, double R, double G, double B)* buffer);
    }
}
