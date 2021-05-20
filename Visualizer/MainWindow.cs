// #define USE_SETTINGS
#define SQ

using System.Drawing.Drawing2D;
using System.Drawing.Imaging;
using System.Drawing;
using System.Windows.Forms;
using System.Threading.Tasks;
using System.Threading;
using System.Diagnostics;
using System;

namespace Visualizer
{
    public partial class MainWindow
        : Form
    {
#if HQ // high quality
        public const int WIDTH = 1920;
        public const int HEIGHT = 1080;
        public const int MAX_ITER = 2048;
        public static int SUBPIXELS = 4;
        public static int SAMPLES = 128;
#elif MQ // medium quality
        public const int WIDTH = 1280;
        public const int HEIGHT = 720;
        public const int MAX_ITER = 48;
        public static int SUBPIXELS = 2;
        public static int SAMPLES = 2;
#else // low quality
        public const int WIDTH = 720;
        public const int HEIGHT = 405;
        public const int MAX_ITER = 8;
        public static int SUBPIXELS = 1;
        public static int SAMPLES = 1;
#endif
        public static RenderMode MODE = RenderMode.Colors;
        public const float FOCAL_LENGTH = 1;
        public static float ZOOM = 2;
        public static float EYE_DIST = 18;
        public static Vec3 EYE = new(0, 0, EYE_DIST);
        public static Vec3 TARGET = new(0, 3, 0);
        public static unsafe void* SCENE = null;

        private static readonly object _mutex = new();
        private static int is_rendering = 0;
        private static int is_pending = 0;


        public unsafe MainWindow()
        {
            InitializeComponent();

            DoubleBuffered = true;
            pictureBox1.SizeMode = PictureBoxSizeMode.Zoom;
            pictureBox1.InterpolationMode = InterpolationMode.NearestNeighbor;

            fixed (void** ptr = &SCENE)
                RayTracer.CreateScene(ptr);
        }

        unsafe ~MainWindow()
        {
            fixed (void** ptr = &SCENE)
                RayTracer.DeleteScene(ptr);
        }

        private void MainWindow_Load(object sender, EventArgs e)
        {
            trackBar1.Value = 47;
            trackBar2.Value = 5;
            trackBar3.Value = (int)(ZOOM * 20);
            numericUpDown1.Value = SAMPLES;
            numericUpDown2.Value = SUBPIXELS;
            comboBox1.DataSource = Enum.GetValues(typeof(RenderMode));
            comboBox1.SelectedItem = MODE;

            trackBar1_Scroll(sender, e);
            trackBar2_Scroll(sender, e);
            trackBar3_Scroll(sender, e);

#if USE_SETTINGS
            Properties.Settings.Default.Upgrade();

            if (Properties.Settings.Default.WindowSize.Width > 0 && Properties.Settings.Default.WindowSize.Height > 0)
                Size = Properties.Settings.Default.WindowSize;

            if (WindowState != FormWindowState.Minimized)
                WindowState = Properties.Settings.Default.WindowState;

            Location = Properties.Settings.Default.WindowLocation;
#endif
        }

        private void MainWindow_FormClosing(object sender, FormClosingEventArgs e)
        {
#if USE_SETTINGS
            Properties.Settings.Default.WindowState = WindowState;
            (Properties.Settings.Default.WindowLocation, Properties.Settings.Default.WindowSize) = WindowState == FormWindowState.Normal ? (Location, Size) : (RestoreBounds.Location, RestoreBounds.Size);
            Properties.Settings.Default.Save();
#endif
        }

        private async void button1_Click(object sender, EventArgs e)
        {
            lock (_mutex)
                if (Interlocked.Exchange(ref is_rendering, 1) != 0)
                {
                    Interlocked.Exchange(ref is_pending, 1);

                    return;
                }

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
                    SubpixelsPerPixel = (ulong)SUBPIXELS,
                };
                (float A, float R, float G, float B)[] buffer = new (float, float, float, float)[config.HorizontalResolution * config.VerticalResolution];
                float progress = 0;
                unsafe void update_image()
                {
                    Bitmap bmp = new Bitmap(WIDTH, HEIGHT, PixelFormat.Format32bppArgb);
                    BitmapData dat = bmp.LockBits(new(0, 0, WIDTH, HEIGHT), ImageLockMode.ReadWrite, bmp.PixelFormat);
                    uint* ptr = (uint*)dat.Scan0;

                    Parallel.For(0, buffer.Length, i =>
                    {
                        float a = buffer[i].A;
                        float r = buffer[i].R;
                        float g = buffer[i].G;
                        float b = buffer[i].B;
                        uint b_a = a < 0 ? 0u : a > 1 ? 255u : (uint)(a * 255);
                        uint b_r = r < 0 ? 0u : r > 1 ? 255u : (uint)(r * 255);
                        uint b_g = g < 0 ? 0u : g > 1 ? 255u : (uint)(g * 255);
                        uint b_b = b < 0 ? 0u : b > 1 ? 255u : (uint)(b * 255);

                        ptr[i] = b_a << 24 | b_r << 16 | b_g << 8 | b_b;
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
                    fixed ((float, float, float, float)* iptr = buffer)
                        RayTracer.RenderImage(SCENE, config, iptr, ref progress);
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

                lock (_mutex)
                {
                    Interlocked.Exchange(ref is_rendering, 0);

                    if (Interlocked.Exchange(ref is_pending, 0) == 0)
                        return;
                }

                Invoke(new MethodInvoker(() => button1_Click(sender, e)));
            });
        }

        private void trackBar1_Scroll(object sender, EventArgs e)
        {
            float angle = (float)(trackBar1.Value * 2 * Math.PI / trackBar1.Maximum);

            EYE.Z = (float)(Math.Cos(angle) * EYE_DIST);
            EYE.X = (float)(Math.Sin(angle) * EYE_DIST);
            label1.Text = $"Hor.Rot.\n{angle * 57.2957795131:F1}°";

            if (checkBox1.Checked)
                button1_Click(sender, e);
        }

        private void trackBar2_Scroll(object sender, EventArgs e)
        {
            EYE.Y = trackBar2.Value;
            label2.Text = $"Hor.Rot.\nY={EYE.Y:F2}m";

            if (checkBox1.Checked)
                button1_Click(sender, e);
        }

        private void trackBar3_Scroll(object sender, EventArgs e)
        {
            ZOOM = trackBar3.Value / 20f;
            label3.Text = $"Zoom\n{ZOOM:F2}x";

            if (checkBox1.Checked)
                button1_Click(sender, e);
        }

        private void comboBox1_SelectedIndexChanged(object sender, EventArgs e)
        {
            Enum.TryParse(comboBox1.SelectedValue.ToString(), out MODE);

            if (checkBox1.Checked)
                button1_Click(sender, e);
        }

        private void numericUpDown1_ValueChanged(object sender, EventArgs e)
        {
            SAMPLES = (int)numericUpDown1.Value;

            if (checkBox1.Checked)
                button1_Click(sender, e);
        }

        private void numericUpDown2_ValueChanged(object sender, EventArgs e)
        {
            SUBPIXELS = (int)numericUpDown2.Value;

            if (checkBox1.Checked)
                button1_Click(sender, e);
        }
    }
}
