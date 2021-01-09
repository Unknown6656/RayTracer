using System.Drawing.Drawing2D;
using System.Windows.Forms;

namespace Visualizer
{
    public class PictureBoxWithInterpolationMode
        : PictureBox
    {
        public InterpolationMode InterpolationMode { get; set; }


        public PictureBoxWithInterpolationMode() => DoubleBuffered = true;

        protected override void OnPaint(PaintEventArgs paintEventArgs)
        {
            paintEventArgs.Graphics.InterpolationMode = InterpolationMode;

            base.OnPaint(paintEventArgs);
        }
    }
}
