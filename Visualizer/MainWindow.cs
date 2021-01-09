//#define HQ

using System;
using System.Diagnostics;
using System.Drawing;
using System.Drawing.Drawing2D;
using System.Drawing.Imaging;
using System.Threading;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace Visualizer
{
    public partial class MainWindow
        : Form
    {
#if HQ
        public const int WIDTH = 1920;
        public const int HEIGHT = 1080;
        public const int MAX_ITER = 2048;
        public const int SAMPLES = 128;
        public const int SUBPIXELS = 4;
#else
        public const int WIDTH = 640 / 16;
        public const int HEIGHT = 360 / 16;
        public const int MAX_ITER = 8;
        public const int SAMPLES = 1;
        public const int SUBPIXELS = 1;
#endif
        public const RenderMode MODE = RenderMode.Colors;
        public const double ZOOM = 1;
        public const double FOCAL_LENGTH = .3;
        public static readonly Vec3 EYE = new(0, 0, 50);
        public static readonly Vec3 TARGET = new(0, 0, 0);


        public MainWindow()
        {
            InitializeComponent();

            DoubleBuffered = true;
            pictureBox1.SizeMode = PictureBoxSizeMode.Zoom;
            pictureBox1.InterpolationMode = InterpolationMode.NearestNeighbor;
        }

        private async void button1_Click(object sender, EventArgs e)
        {
            button1.Enabled = false;

            await Task.Factory.StartNew(delegate
            {
                RenderConfiguration config = new RenderConfiguration
                {
                    Camera = new CameraConfiguration
                    {
                        Position = EYE,
                        LookAt = TARGET,
                        ZoomFactor = ZOOM,
                        FocalLength = FOCAL_LENGTH,
                    },
                    RenderMode = MODE,
                    HorizontalResolution = WIDTH,
                    VerticalResolution = HEIGHT,
                    MaximumIterationCount = MAX_ITER,
                    SamplesPerSubpixel = SAMPLES,
                    SubpixelsPerPixel = SUBPIXELS,
                };
                var buffer = new (double A, double R, double G, double B)[config.HorizontalResolution * config.VerticalResolution];
                unsafe void update_image()
                {
                    Bitmap bmp = new Bitmap(WIDTH, HEIGHT, PixelFormat.Format32bppArgb);
                    BitmapData dat = bmp.LockBits(new(0, 0, WIDTH, HEIGHT), ImageLockMode.ReadWrite, bmp.PixelFormat);
                    uint* ptr = (uint*)dat.Scan0;

                    Parallel.For(0, buffer.Length, i =>
                    {
                        double a = buffer[i].A;
                        double r = buffer[i].R;
                        double g = buffer[i].G;
                        double b = buffer[i].B;
                        uint b_a = a < 0 ? 0 : a > 1 ? 255 : (byte)(a * 255);
                        uint b_r = r < 0 ? 0 : r > 1 ? 255 : (byte)(r * 255);
                        uint b_g = g < 0 ? 0 : g > 1 ? 255 : (byte)(g * 255);
                        uint b_b = b < 0 ? 0 : b > 1 ? 255 : (byte)(b * 255);

                        ptr[i] = b_a << 24
                               | b_r << 16
                               | b_g << 8
                               | b_b;
                    });

                    bmp.UnlockBits(dat);
                    Invoke(new MethodInvoker(delegate
                    {
                        using Image? old = pictureBox1.Image;

                        pictureBox1.Image = bmp;
                        old?.Dispose();
                    }));
                }

                Stopwatch sw = new();
                sw.Start();

                bool finished = false;
                Task updater = Task.Factory.StartNew(async delegate
                {
                    while (!finished)
                    {
                        update_image();

                        await Task.Delay(1);
                    }
                });

                unsafe
                {
                    fixed ((double, double, double, double)* iptr = buffer)
                        RayTracer.RenderImage(config, iptr);
                }

                finished = true;

                sw.Stop();
                update_image();

                Invoke(new MethodInvoker(delegate
                {
                    button1.Enabled = true;
                    button1.Text = $"(RE)RENDER                    previous: {sw.ElapsedMilliseconds:F4}ms";
                    pictureBox1.Image?.Save("./render.png", ImageFormat.Png);
                }));
            });
        }
    }
}
