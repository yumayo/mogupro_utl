using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace fmakeGUI
{
    public partial class Form2 : Form
    {
        string currentNamespace;

        public Form2(string currentNamespace)
        {
            InitializeComponent();

            StartPosition = FormStartPosition.CenterParent;

            this.currentNamespace += currentNamespace;

            UpdateNamespace();

            ActiveControl = textBox1;
        }

        public string ResultText { get; private set; }

        private void button1_Click(object sender, EventArgs e)
        {
            DialogResult = DialogResult.OK;
            ResultText = textBox1.Text;
            Close();
        }

        private void button2_Click(object sender, EventArgs e)
        {
            DialogResult = DialogResult.Cancel;
            Close();
        }

        private void UpdateNamespace()
        {
            var resultNamespace = this.currentNamespace + textBox1.Text;
            var ns = resultNamespace.Split('\\');
            string result = "::";
            foreach (var n in ns)
            {
                if (n.Length != 0)
                {
                    result += n + "::";
                }
            }
            label2.Text = result;
        }

        private void textBox1_TextChanged(object sender, EventArgs e)
        {
            UpdateNamespace();
        }
    }
}
