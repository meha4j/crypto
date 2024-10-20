package rot

import (
	"crypto/rand"
	"fmt"
	"os"

	"github.com/meha4j/crypto/rot"
	"github.com/spf13/cobra"
)

var KgenCmd = &cobra.Command{
	Use:   "kgen",
	Short: "Generate private key using Fisher-Yates shuffle.",
	RunE: func(cmd *cobra.Command, args []string) error {
		fout := os.Stdout

		if cmd.Flags().Changed("out") {
			f, err := os.Create(nout)

			if err != nil {
				return err
			}

			fout = f
		}

		key, err := rot.GenerateKey(rand.Reader)

		if err != nil {
			return err
		}

		fmt.Fprintln(fout, key)
		fout.Close()

		return nil
	},
}

func init() {
	KgenCmd.Flags().StringVarP(&nout, "out", "o", "", "output file, default: stdout")
}
