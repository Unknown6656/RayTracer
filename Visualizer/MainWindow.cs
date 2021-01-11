#define SQ

using System;
using System.Diagnostics;
using System.Drawing;
using System.Drawing.Drawing2D;
using System.Drawing.Imaging;
using System.Threading.Tasks;
using System.Windows.Forms;


namespace Visualizer
{
    public partial class MainWindow
        : Form
    {
#if HQ // high quality
        public const int WIDTH = 1920;
        public const int HEIGHT = 1080;
        public const int MAX_ITER = 2048;
        public const int SUBPIXELS = 4;
        public static int SAMPLES = 128;
#elif MQ // medium quality
        public const int WIDTH = 1280;
        public const int HEIGHT = 720;
        public const int MAX_ITER = 48;
        public const int SUBPIXELS = 2;
        public static int SAMPLES = 2;
#else // low quality
        public const int WIDTH = 640;
        public const int HEIGHT = 360;
        public const int MAX_ITER = 8;
        public const int SUBPIXELS = 1;
        public static int SAMPLES = 2;
#endif
        public static RenderMode MODE = RenderMode.RenderTime;
        public const double FOCAL_LENGTH = .3;
        public static double ZOOM = 1;
        public static Vec3 EYE = new(0, 0, 10);
        public static Vec3 TARGET = new(0, 0, 0);
        public static Scene SCENE;


        public unsafe MainWindow()
        {
            InitializeComponent();

            DoubleBuffered = true;
            pictureBox1.SizeMode = PictureBoxSizeMode.Zoom;
            pictureBox1.InterpolationMode = InterpolationMode.NearestNeighbor;

            RayTracer.CreateScene(out SCENE);
        }

        private void MainWindow_Load(object sender, EventArgs e)
        {
            trackBar1.Value = 32;
            trackBar2.Value = 5;
            trackBar3.Value = 60;
            numericUpDown1.Value = SAMPLES;
            comboBox1.DataSource = Enum.GetValues(typeof(RenderMode));
            comboBox1.SelectedItem = MODE;

            trackBar1_Scroll(sender, e);
            trackBar2_Scroll(sender, e);
            trackBar3_Scroll(sender, e);
        }

        private async void button1_Click(object sender, EventArgs e)
        {
            button1.Enabled = false;
            progressBar1.Value = 0;

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
                    SamplesPerSubpixel = (ulong)SAMPLES,
                    SubpixelsPerPixel = SUBPIXELS,
                };
                var buffer = new (double A, double R, double G, double B)[config.HorizontalResolution * config.VerticalResolution];
                double progress = 0;
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

                        button1.Text = $"{progress * 100:F4} %";
                        progressBar1.Value = (int)(progress * progressBar1.Maximum);
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
                        RayTracer.RenderImage(ref SCENE, config, iptr, ref progress);
                }

                finished = true;

                sw.Stop();
                update_image();

                Invoke(new MethodInvoker(delegate
                {
                    button1.Enabled = true;
                    progressBar1.Value = progressBar1.Maximum;
                    button1.Text = $"(RE)RENDER                    previous: {sw.ElapsedMilliseconds:F4}ms";
                    pictureBox1.Image?.Save("./render.png", ImageFormat.Png);
                }));
            });
        }

        private void trackBar1_Scroll(object sender, EventArgs e)
        {
            double angle = trackBar1.Value * 2 * Math.PI / trackBar1.Maximum;

            EYE.Z = Math.Cos(angle) * 10;
            EYE.X = Math.Sin(angle) * 10;
            label1.Text = $"Hor.Rot.\n{angle*57.2957795131:F1}°";
        }

        private void trackBar2_Scroll(object sender, EventArgs e)
        {
            EYE.Y = trackBar2.Value;
            label2.Text = $"Hor.Rot.\nY={EYE.Y:F2}m";
        }

        private void trackBar3_Scroll(object sender, EventArgs e)
        {
            ZOOM = trackBar3.Value / 20d;
            label3.Text = $"Zoom\n{ZOOM:F2}x";
        }

        private void comboBox1_SelectedIndexChanged(object sender, EventArgs e) => Enum.TryParse(comboBox1.SelectedValue.ToString(), out MODE);

        private void numericUpDown1_ValueChanged(object sender, EventArgs e) => SAMPLES = (int)numericUpDown1.Value;
    }
}
