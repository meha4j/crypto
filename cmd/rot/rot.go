package rot

import (
	"bufio"
	"fmt"
	"io"
	"os"

	"github.com/meha4j/crypto/rot"
	"github.com/spf13/cobra"
)

var (
	nin  string
	nout string
	nkey string
	dec  bool

	RotCmd = &cobra.Command{
		Use:   "rot",
		Short: "Simple substitution algorithm.",
		RunE: func(cmd *cobra.Command, args []string) error {
			fin := os.Stdin

			if cmd.Flags().Changed("in") {
				f, err := os.Open(nin)

				if err != nil {
					return err
				}

				fin = f
				defer fin.Close()
			}

			fout := os.Stdout

			if cmd.Flags().Changed("out") {
				f, err := os.Create(nout)

				if err != nil {
					return err
				}

				fout = f
				defer fout.Close()
			}

			if !cmd.Flags().Changed("key") {
				return fmt.Errorf("key missed.")
			}

			fkey, err := os.Open(nkey)

			if err != nil {
				return err
			}

			defer fkey.Close()

			key, err := rot.ReadKey(bufio.NewReader(fkey))

			if err != nil {
				return err
			}

			var reader io.Reader

			if dec {
				decr := rot.NewDecrypter(bufio.NewReader(fin), key.Public())
				reader = &decr
			} else {
				encr := rot.NewEncrypter(bufio.NewReader(fin), key)
				reader = &encr
			}

			_, err = io.Copy(fout, reader)

			if err != nil {
				return err
			}

			return nil
		},
	}
)

func init() {
	RotCmd.AddCommand(KgenCmd)

	RotCmd.Flags().StringVarP(&nin, "in", "i", "", "input file, default: stdin")
	RotCmd.Flags().StringVarP(&nout, "out", "o", "", "output file, default: stdout")
	RotCmd.Flags().StringVarP(&nkey, "key", "k", "", "key file (required)")
	RotCmd.Flags().BoolVarP(&dec, "dec", "d", false, "decrypt given input")

	RotCmd.MarkFlagRequired("key")
}
