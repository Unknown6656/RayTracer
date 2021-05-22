using System.Windows.Forms;
using System.Threading;

using Visualizer;


Thread thread = new(() =>
{
    Application.SetHighDpiMode(HighDpiMode.SystemAware);
    Application.EnableVisualStyles();
    Application.SetCompatibleTextRenderingDefault(false);
    Application.Run(new MainWindow());
});
thread.SetApartmentState(ApartmentState.STA);
thread.Start();
thread.Join();
